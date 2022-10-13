use std::collections::HashMap;
use std::fmt;
use std::io::Read;
use std::sync::Mutex;
use std::time::{Duration, Instant};

use lazy_static::lazy_static;
use paho_mqtt::{AsyncClient, Message, MessageBuilder};
use serde_json::{json, Value};

use crate::conf::get_device;
use crate::conf::getdevicename;
use crate::conn::QOS::{QOS0, QOS1};

const ONLINE: &'static str = "online";
const OFFLINE: &'static str = "offline";
const LOCATION: &'static str = "Office";

pub enum QOS {
    QOS0 = 0,
    QOS1 = 1,
    // QOS2 = 2,
}

impl fmt::Display for QOS {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        match *self {
            QOS::QOS0 => write!(f, "QOS0"),
            QOS::QOS1 => write!(f, "QOS1"),
            // QOS::QOS2 => write!(f, "QOS2"),
        }
    }
}

pub struct TASK {
    pub topic: String,
    pub func: fn(),
}

pub struct SENSOR {
    pub topic: String,
    pub func: fn(now: &Instant) -> Result<String, ()>,
}
lazy_static! {
    static ref TASK_LIST: Mutex<Vec<TASK>> = Mutex::new(vec![]);
}
lazy_static! {
    static ref SENSOR_LIST: Mutex<Vec<SENSOR>> = Mutex::new(vec![]);
}

pub fn conn_recvmsg(
    _client: &AsyncClient, message_opt: Option<Message>,
) {
    if message_opt.is_none() {
        return;
    }
    let message = message_opt.unwrap();
    let mut payload: String = String::new();
    message.payload().read_to_string(&mut payload).unwrap();
    let topic: String = String::from(message.topic());

    println!("Message arrived");
    println!("     topic: {}", topic);
    println!("     message: {}", payload);
    for task in TASK_LIST.lock().unwrap().iter() {
        if topic == task.topic
        {
            (task.func)();
            break;
        }
    }
}

#[no_mangle]
pub fn connlost(
    _context: AsyncClient,
    cause: &str,
) {
    println!("\nConnection lost");
    println!("     cause: {cause}");
}

pub fn get_availability_topic(
    location: &str,
    hostname: &String,
) -> Result<String, ()> {
    let topic = format!("{location}/{hostname}/availability");
    if topic.len() > 65536 {
        panic!("Formatted topic is an invalid TOPIC length");
    }
    return Ok(topic);
}

pub fn conn_register_available(client: &AsyncClient) -> Result<(), ()> {
    let hostname: String = getdevicename().expect("Failed to get device name");
    let availability_topic = get_availability_topic(
        &LOCATION,
        &hostname,
    ).expect("Failed to get availability topic");
    conn_publish(
        &client,
        availability_topic,
        String::from(ONLINE),
        QOS1,
        false,
    ).expect("Failed to set availablility");

    return Ok(());
}

pub fn conn_init(
    address: String,
) -> Result<AsyncClient, ()> {
    let hostname = getdevicename().expect("Failed to get device name");
    let id = format!("pc_manager_{hostname}");
    let create_opts = paho_mqtt::CreateOptionsBuilder::new()
        .server_uri(address)
        .client_id(id)
        .finalize();
    let client = paho_mqtt::AsyncClient::new(create_opts)
        .expect("Error creating the client");

    let availability = get_availability_topic(&LOCATION, &hostname).unwrap();
    let lwt = paho_mqtt::MessageBuilder::new()
        .topic(availability)
        .payload(OFFLINE)
        .finalize();
    let conn_opts = paho_mqtt::ConnectOptionsBuilder::new()
        .keep_alive_interval(Duration::from_secs(30))
        .mqtt_version(paho_mqtt::MQTT_VERSION_3_1_1)
        .clean_session(true)
        .will_message(lwt)
        .finalize();
    client.set_message_callback(conn_recvmsg);
    client.connect(conn_opts).wait().unwrap();
    return Ok(client);
}

#[no_mangle]
pub fn conn_subscribe(
    client: &AsyncClient,
    topic: String,
    qos: QOS,
) -> Result<(), ()> {
    println!("Subscribing to topic [{topic}] using [{qos}]");
    client.subscribe(topic, qos as i32).wait().expect("Failed to subscribe");

    return Ok(());
}

pub fn conn_publish(
    client: &AsyncClient,
    topic: String,
    value: String,
    qos: QOS,
    retained: bool,
) -> Result<(), ()> {
    println!("Publishing to topic [{topic}] with [{qos}]");

    let message = MessageBuilder::new()
        .topic(topic)
        .payload(value)
        .qos(qos as i32)
        .retained(retained)
        .finalize();
    client.publish(message).wait().unwrap();
    return Ok(());
}

#[no_mangle]
pub fn conn_register_task(
    client: &AsyncClient,
    task_name: String,
    func: fn(),
) -> Result<(), ()> {
    let hostname = getdevicename().unwrap();
    let command_topic = format!("{LOCATION}/{hostname}/command/{task_name}");
    let availability_topic = get_availability_topic(&LOCATION, &hostname);
    let unique_id = format!("{}-{task_name}", &hostname);
    let disco_string = format!("homeassistant/button/{task_name}/{hostname}/config");

    let object = json!({
        "name": task_name,
        "object_id": unique_id,
        "command_topic": command_topic,
        "availability_topic": availability_topic,
        "unique_id": unique_id,
        "device": get_device(&hostname, &LOCATION)
    });

    conn_publish(
        &client,
        disco_string,
        object.to_string(),
        QOS0,
        true,
    ).unwrap();
    conn_subscribe(
        client,
        command_topic,
        QOS1,
    ).unwrap();

    let task = TASK { topic: task_name, func };
    TASK_LIST.lock().unwrap().push(task);
    return Ok(());
}

#[no_mangle]
pub fn conn_register_sensor(
    client: &AsyncClient,
    sensor_name: String,
    unit: Option<String>,
    class: Option<String>,
    func: fn(now: &Instant) -> Result<String, ()>,
) -> Result<(), ()> {
    let hostname = getdevicename().unwrap();
    let state_topic = format!("{LOCATION}/{hostname}/sensor/{sensor_name}");
    let availability_topic = get_availability_topic(&LOCATION, &hostname).unwrap();
    let unique_id = format!("{hostname}-{sensor_name}");
    let disco_string = format!("homeassistant/sensor/{sensor_name}/{hostname}/config");


    let mut object: HashMap<&str, Value> = HashMap::from([
        ("name", Value::String(sensor_name.clone())),
        ("object_id", Value::String(unique_id.clone())),
        ("state_topic", Value::String(state_topic.clone())),
        ("availability_topic", Value::String(availability_topic)),
        ("unique_id", Value::String(unique_id)),
        ("device", get_device(&hostname, &LOCATION))
    ]);
    if unit.is_some() {
        object.insert("unit_of_measurement", unit.unwrap().parse().unwrap());
    }
    if class.is_some() {
        object.insert("device_class", class.unwrap().parse().unwrap());
    }

    conn_publish(
        &client,
        disco_string,
        json!(object).to_string(),
        QOS0,
        true,
    ).expect("Failed to publish");
    let sensor: SENSOR = SENSOR { topic: state_topic, func: func };
    SENSOR_LIST.lock().unwrap().push(sensor);

    return Ok(());
}

#[no_mangle]
pub fn conn_deregister_task(
    client: AsyncClient,
    task_name: String,
) -> Result<(), ()> {
    let hostname = getdevicename().unwrap();
    let command_topic = format!("{LOCATION}/{hostname}/command/{task_name}");
    let disco_string = format!("homeassistant/button/{task_name}/{hostname}/config");


    conn_publish(
        &client,
        disco_string,
        String::from(""),
        QOS0,
        true,
    ).unwrap();
    client.unsubscribe(command_topic);
    //TODO: taskList.remove(task)
    return Ok(());
}

#[no_mangle]
pub fn process_sensors(client: &AsyncClient) -> Result<(), ()> {
    let now = Instant::now();
    for sensor in SENSOR_LIST.lock().unwrap().iter() {
        let data = (sensor.func)(&now);
        if data.is_ok() {
            conn_publish(
                &client,
                sensor.topic.clone(),
                data.unwrap(),
                QOS0,
                true,
            ).unwrap();
        }
    }
    return Ok(());
}

#[no_mangle]
pub fn conn_cleanup(client: AsyncClient) -> Result<(), ()> {
    client.disconnect(None);
    return Ok(());
}

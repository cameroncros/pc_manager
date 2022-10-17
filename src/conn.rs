use std::collections::HashMap;
use std::sync::Mutex;
use std::time::Instant;

use rumqttc::{AsyncClient, ClientError, EventLoop, LastWill, MqttOptions, QoS};
use serde_json::{json, Value};

use crate::conf::get_device;
use crate::conf::getdevicename;

const ONLINE: &'static str = "online";
const OFFLINE: &'static str = "offline";
const LOCATION: &'static str = "Office";

pub struct TASK {
    pub topic: String,
    pub func: fn(),
}

pub struct SENSOR {
    pub topic: String,
    pub func: fn(now: &Instant) -> Result<String, ()>,
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

pub struct Connection {
    task_list: Mutex<Vec<TASK>>,
    sensor_list: Mutex<Vec<SENSOR>>,
    client: AsyncClient,
    _eventloop: EventLoop,
    connected: bool,
}

impl Connection {}

impl Connection {
    pub async fn new(
        address: String,
    ) -> Self {
        let hostname = getdevicename().expect("Failed to get device name");
        let id = format!("pc_manager_{hostname}");

        let mut create_opts = MqttOptions::new(id, address, 1883);
        let availability = get_availability_topic(&LOCATION, &hostname).unwrap();
        create_opts.set_last_will(LastWill::new(availability, OFFLINE, QoS::AtLeastOnce, false));
        let (client, eventloop) = rumqttc::AsyncClient::new(create_opts, 100);

        return Self {
            task_list: Mutex::new(vec![]),
            sensor_list: Mutex::new(vec![]),
            client,
            _eventloop: eventloop,
            connected: true,
        };
    }

    pub async fn run(&mut self) {
        // Iterate to poll the eventloop for connection progress

        loop {
            let i = self._eventloop.poll().await;
            if i.is_err() {
                println!("Failed with: {:?}", i.unwrap_err());
                break;
            } else {
                println!("Notification = {:?}", i.unwrap());
            }

            self.process_sensors().await.unwrap();
        }
    }

// pub fn recvmsg(
//     _&self, message_opt: Option<Message>,
// ) {
//     if message_opt.is_none() {
//         return;
//     }
//     let message = message_opt.unwrap();
//     let mut payload: String = String::new();
//     message.payload().read_to_string(&mut payload).unwrap();
//     let topic: String = String::from(message.topic());
//
//     println!("Message arrived");
//     println!("     topic: {}", topic);
//     println!("     message: {}", payload);
//     for task in task_list.lock().unwrap().iter() {
//         if topic == task.topic
//         {
//             (task.func)();
//             break;
//         }
//     }
// }

    pub async fn register_available(&self) -> Result<(), ()> {
        let hostname: String = getdevicename().expect("Failed to get device name");
        let availability_topic = get_availability_topic(
            &LOCATION,
            &hostname,
        ).expect("Failed to get availability topic");
        self.publish(
            availability_topic,
            String::from(ONLINE),
            QoS::AtLeastOnce,
            false,
        ).await.expect("Failed to set availablility");

        return Ok(());
    }

    pub async fn subscribe(
        &self,
        topic: String,
        qos: QoS,
    ) -> Result<(), ()> {
        println!("Subscribing to topic [{topic}] using [{:?}]", qos);
        self.client.subscribe(topic, qos).await.expect("Failed to subscribe");

        return Ok(());
    }

    pub async fn publish(
        &self,
        topic: String,
        value: String,
        qos: QoS,
        retained: bool,
    ) -> Result<(), ClientError> {
        println!("Publishing to topic [{topic}] with [{:?}]", qos);

        return self.client.publish(topic, qos, retained, value).await;
    }

    pub async fn register_task(
        &self,
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

        self.publish(
            disco_string,
            object.to_string(),
            QoS::AtLeastOnce,
            true,
        ).await.unwrap();
        self.subscribe(
            command_topic,
            QoS::AtLeastOnce,
        ).await.unwrap();

        let task = TASK { topic: task_name, func };
        self.task_list.lock().unwrap().push(task);
        return Ok(());
    }

    pub async fn register_sensor(
        &self,
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

        self.publish(
            disco_string,
            json!(object).to_string(),
            QoS::AtLeastOnce,
            true,
        ).await.unwrap();
        let sensor: SENSOR = SENSOR { topic: state_topic, func };
        self.sensor_list.lock().unwrap().push(sensor);

        return Ok(());
    }

    pub async fn process_sensors(&self) -> Result<(), ()> {
        let now = Instant::now();
        for sensor in self.sensor_list.lock().unwrap().iter() {
            let data = (sensor.func)(&now);
            if data.is_ok() {
                self.publish(
                    sensor.topic.clone(),
                    data.unwrap(),
                    QoS::AtLeastOnce,
                    true,
                ).await.unwrap();
            }
        }
        return Ok(());
    }

    pub async fn cleanup(&mut self) -> Result<(), ClientError> {
        self.connected = false;
        return self.client.disconnect().await;
    }
}

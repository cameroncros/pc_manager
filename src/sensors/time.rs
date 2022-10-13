use std::time::Instant;
use chrono::{Local, SubsecRound};

pub fn sensor_time(now: &Instant) -> Result<String, ()> {
    if now.elapsed().as_secs() % 60 != 0 {
        return Err(());
    }
    let date = Local::now().trunc_subsecs(0);
    return Ok(format!("{date}"));
}

#[cfg(test)]
mod tests {
    use std::time::Instant;
    use regex::Regex;
    use crate::sensors::time::sensor_time;

    #[test]
    fn test_sensor_time() {
        let res = sensor_time(&Instant::now());
        assert_eq!(true, res.is_ok());
        let version = res.unwrap();
        let re = Regex::new(r"[0-9]{4}-[0-9]{2}-[0-9]{2} [0-9]{2}:[0-9]{2}:[0-9]{2} \+[0-9]{2}:[0-9]{2}").unwrap();
        assert_eq!(true, re.is_match(&version));
    }
}

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
    use crate::sensors::time::sensor_time;

    #[test]
    fn test_check_for_update() {
        assert_eq!(Ok(String::from("")), sensor_time(Instant::now()));
    }
}

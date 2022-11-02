#[cfg(target_os = "linux")]
pub(crate) mod update {
    use std::fs::{File, Permissions, set_permissions};
    use std::io::Write;
    use std::os::unix::fs::PermissionsExt;
    use std::process::{Command, exit};

    use nix::sys::wait::waitpid;
    use nix::unistd::{chdir, fork, ForkResult, getuid, setuid, User};
    use nix::unistd::seteuid;

    use reqwest::blocking::ClientBuilder;
    use reqwest::redirect::Policy;
    use serde_json::Value;

    pub fn build_package(
        update_url: String,
    ) -> Result<(), String> {
        let cur_uid = getuid();
        if cur_uid.is_root() {
            let user = User::from_name("nobody").unwrap().unwrap();
            setuid(user.uid).unwrap();
            seteuid(user.uid).unwrap();
        }
        println!("Downloading: {update_url}");
        let client = ClientBuilder::new().user_agent("pc_manager/1.0").redirect(Policy::limited(5)).build().unwrap();
        let resp = client.get(update_url).send().unwrap();
        {
            let mut file = File::create("PKGBUILD").unwrap();
            file.write_all(resp.bytes().unwrap().as_ref()).unwrap();
        }
        let result = Command::new("makepkg").output();
        if result.is_ok() {
            return Ok(());
        }
        return Err(result.err().unwrap().to_string());
    }

    pub fn install_update_arch(
        update_url: String,
    ) -> Result<(), ()> {
        let tmpdir = tempdir::TempDir::new("pc_manager").unwrap();
        let permissions = Permissions::from_mode(0o777);
        set_permissions(tmpdir.path(), permissions).unwrap();
        chdir(tmpdir.path()).unwrap();

        match unsafe { fork() } {
            Ok(ForkResult::Parent { child, .. }) => {
                println!("Continuing execution in parent process, new child has pid: {}", child);
                waitpid(child, None).unwrap();
            }
            Ok(ForkResult::Child) => {
                build_package(update_url).unwrap();
                exit(0);
            }
            Err(_) => println!("Fork failed"),
        }
        Command::new("pacman -U --noconfirm *.tar*").spawn().unwrap();
        Command::new("systemctl restart pc_manager").spawn().unwrap();
        return Ok(());
    }

    pub fn get_update_url() -> Result<String, String> {
        let client = ClientBuilder::new().user_agent("pc_manager/1.0").redirect(Policy::limited(5)).build().unwrap();
        let resp = client.get("https://api.github.com/repos/cameroncros/pc_manager/releases").send().unwrap();
        if resp.status().as_u16() != 200 {
            return Err(String::from("Failed to get releases"));
        }
        let response_bytes = resp.bytes().unwrap();
        let jsdata: Value = serde_json::from_slice(response_bytes.as_ref()).unwrap();
        if !jsdata.is_array() {
            return Err(String::from("Not a list of releases"));
        }
        let releases = jsdata.as_array().unwrap();

        for release in releases {
            let assets_data = release.get("assets").unwrap();
            if !assets_data.is_array()
            {
                return Err(String::from("Not a list of assets"));
            }
            let assets = assets_data.as_array().unwrap();
            for asset in assets {
                let asset_name = asset.get("name").unwrap().as_str().unwrap();
                if asset_name == "PKGBUILD" {
                    let asset_url = asset.get("browser_download_url").unwrap().as_str().unwrap();
                    return Ok(String::from(asset_url));
                }
            }
        }

        return Err(String::from("Failed to find release URL"));
    }

    pub fn task_update() {
        let update_url = get_update_url().unwrap();
        install_update_arch(update_url).unwrap();
    }

    #[cfg(test)]
    mod tests {
        use std::fs::{read_dir, remove_file};
        use regex::Regex;
        use crate::tasks::update::update::{build_package, get_update_url, task_update};

        #[test]
        #[ignore]
        fn test_build_package() {
            let update_url = get_update_url().unwrap();
            let res = build_package(update_url);
            assert_eq!(true, res.is_ok());

            let mut found = false;
            let paths = read_dir("./").unwrap();
            for path in paths {
                let path_str = path.unwrap().path().display().to_string();
                println!("Name: {path_str}");
                if path_str.ends_with(".tar.xz") {
                    found = true;
                }
            }
            assert_eq!(true, found);

            remove_file("PKGBUILD").unwrap();
        }

        #[test]
        #[ignore]
        fn test_task_update() {
            task_update();
        }

        #[test]
        fn test_get_update_url() {
            let result = get_update_url();
            assert_eq!(true, result.is_ok());
            let re = Regex::new(r"https://github.com/cameroncros/pc_manager/releases/download/.*/PKGBUILD").unwrap();
            assert_eq!(true, re.is_match(result.unwrap().as_str()));
        }
    }
}

#[cfg(target_os = "windows")]
pub(crate) mod update {
    pub fn download_package(
        update_url: String,
    ) -> Result<(), ()> {
        let client = ClientBuilder::new().user_agent("pc_manager/1.0").redirect(Policy::limited(5)).build().unwrap();
        let resp = client.get(update_url).send().unwrap();
        {
            let mut file = File::create("pc_manager.msi").unwrap();
            file.write_all(resp.bytes().unwrap().as_ref()).unwrap();
        }
        return Ok(());
    }

    pub fn install_update_win(
        update_url: String,
    ) -> Result<(), ()> {
        let tmpdir = tempdir::TempDir::new("pc_manager").unwrap();
        let permissions = Permissions::from_mode(0o777);
        set_permissions(tmpdir.path(), permissions).unwrap();
        chdir(tmpdir.path()).unwrap();

        download_package(update_url).unwrap();
        Command::new("pc_manager.msi").spawn().unwrap();
        return Ok(());
    }

    pub fn task_update() {
        let update_url = get_update_url().unwrap();
        install_update_win(update_url).unwrap();
    }

    pub fn get_update_url() -> Result<String, String> {
        let client = ClientBuilder::new().user_agent("pc_manager/1.0").redirect(Policy::limited(5)).build().unwrap();
        let resp = client.get("https://api.github.com/repos/cameroncros/pc_manager/releases").send().unwrap();
        if resp.status().as_u16() != 200 {
            return Err(String::from("Failed to get releases"));
        }
        let response_bytes = resp.bytes().unwrap();
        let jsdata: Value = serde_json::from_slice(response_bytes.as_ref()).unwrap();
        if !jsdata.is_array() {
            return Err(String::from("Not a list of releases"));
        }
        let releases = jsdata.as_array().unwrap();

        for release in releases {
            let assets_data = release.get("assets").unwrap();
            if !assets_data.is_array()
            {
                return Err(String::from("Not a list of assets"));
            }
            let assets = assets_data.as_array().unwrap();
            for asset in assets {
                let asset_name = asset.get("name").unwrap().as_str().unwrap();
                if asset_name.ends_with(".msi") {
                    let asset_url = asset.get("url").unwrap().as_str().unwrap();
                    return Ok(String::from(asset_url));
                }
            }
        }

        return Err(String::from("Failed to find release URL"));
    }

    #[cfg(test)]
    mod tests {
        use regex::Regex;
        use crate::tasks::update::update::{get_update_url, task_update};

        #[test]
        #[ignore]
        fn test_task_update() {
            task_update();
        }

        #[test]
        fn test_get_update_url() {
            let result = get_update_url();
            assert_eq!(true, result.is_ok());
            let re = Regex::new(r"https://github\.com/cameroncros/pc_manager/releases/download/.*\.msi").unwrap();
            assert_eq!(true, re.is_match(result.unwrap().as_str()));
        }
    }
}

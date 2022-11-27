#[cfg(target_os = "linux")]
pub(crate) mod update {
    use std::fs::{write, Permissions, set_permissions};
    use std::os::unix::fs::PermissionsExt;
    use std::process::{Command, exit, Output};

    use nix::sys::wait::{waitpid, WaitStatus};
    use nix::unistd::{chdir, fork, ForkResult, getuid, setuid, User};
    use nix::unistd::seteuid;

    use reqwest::blocking::ClientBuilder;
    use reqwest::redirect::Policy;
    use serde_json::Value;

    pub fn build_package(
        update_url: String,
    ) -> std::io::Result<Output> {
        let cur_uid = getuid();
        if cur_uid.is_root() {
            let user = User::from_name("nobody").unwrap().unwrap();
            setuid(user.uid).unwrap();
            seteuid(user.uid).unwrap();
        }
        println!("Downloading: {update_url}");
        let client = ClientBuilder::new().user_agent("pc_manager/1.0").redirect(Policy::limited(5)).build().unwrap();
        let resp = client.get(update_url).send().unwrap();
        write("PKGBUILD", resp.bytes().unwrap()).unwrap();
        let result = Command::new("makepkg").output();
        let output = result.as_ref().unwrap();
        println!("status: {}", output.status);
        println!("stdout: {}", String::from_utf8_lossy(&output.stdout));
        println!("stderr: {}", String::from_utf8_lossy(&output.stderr));
        return result;
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
                println!("Continuing execution in parent process, new child has pid: {child}");
                match { waitpid(child, None).unwrap() } {
                    WaitStatus::Exited(_, 0) => {
                        println!("Build successful.");
                    },
                    WaitStatus::Exited(_, retcode) => {
                        println!("Build failed with: [{retcode}]");
                    },
                    _ => {
                        println!("Something went wrong!");
                    }
                }
            }
            Ok(ForkResult::Child) => {
                let output = build_package(update_url).unwrap();
                exit(output.status.code().unwrap());
            }
            Err(_) => println!("Fork failed"),
        }
        {
            let output = Command::new("pkexec").args(["pacman", "-U", "--noconfirm", "*.tar*"]).output().unwrap();
            println!("status: {}", output.status);
            println!("stdout: {}", String::from_utf8_lossy(&output.stdout));
            println!("stderr: {}", String::from_utf8_lossy(&output.stderr));
            assert!(output.status.success());
        }
        {
            let output = Command::new("systemctl").args(["restart", "pc_manager"]).output().unwrap();
            println!("status: {}", output.status);
            println!("stdout: {}", String::from_utf8_lossy(&output.stdout));
            println!("stderr: {}", String::from_utf8_lossy(&output.stderr));
            assert!(output.status.success());
        }

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
        use std::env::current_dir;
        use std::fs::{Permissions, read_dir, remove_dir_all, set_permissions, copy};
        use std::os::unix::fs::PermissionsExt;
        use std::process::Command;
        use nix::unistd::chdir;
        use regex::Regex;
        use crate::tasks::update::update::{build_package, get_update_url, task_update};

        #[test]
        #[ignore]
        fn test_pkgbuild() {
            let tmpdir = tempdir::TempDir::new("pc_manager").unwrap();
            println!("TMPDIR: [{}]", tmpdir.as_ref().display());
            let permissions = Permissions::from_mode(0o777);
            set_permissions(tmpdir.path(), permissions).unwrap();

            let tmp_pkgbuild = format!("{}/PKGBUILD", tmpdir.path().display());
            copy("install/PKGBUILD", tmp_pkgbuild).unwrap();

            let original_dir = current_dir().unwrap();
            chdir(tmpdir.path()).unwrap();

            let output = Command::new("makepkg").args(["--nocheck"]).output().unwrap();
            println!("status: {}", output.status);
            println!("stdout: {}", String::from_utf8_lossy(&output.stdout));
            println!("stderr: {}", String::from_utf8_lossy(&output.stderr));
            assert!(output.status.success());

            let mut found = false;
            let paths = read_dir("./").unwrap();
            for path in paths {
                let path_str = path.unwrap().path().display().to_string();
                println!("Name: {path_str}");
                if path_str.contains(".pkg.") {
                    found = true;
                }
            }
            assert_eq!(true, found);

            chdir(original_dir.as_path()).unwrap();
            remove_dir_all(tmpdir.path()).unwrap();
        }

        #[test]
        #[ignore]
        fn test_build_package() {
            let tmpdir = tempdir::TempDir::new("pc_manager").unwrap();
            println!("TMPDIR: [{}]", tmpdir.as_ref().display());
            let permissions = Permissions::from_mode(0o777);
            set_permissions(tmpdir.path(), permissions).unwrap();
            let original_dir = current_dir().unwrap();
            chdir(tmpdir.path()).unwrap();

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

            chdir(original_dir.as_path()).unwrap();
            remove_dir_all(tmpdir.path()).unwrap();
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
    use std::env::{current_dir, set_current_dir};
    use std::fs::write;
    use std::process::Command;
    use reqwest::blocking::ClientBuilder;
    use reqwest::redirect::Policy;
    use serde_json::Value;

    pub fn download_package(
        update_url: String,
    ) -> Result<(), ()> {
        let client = ClientBuilder::new().user_agent("pc_manager/1.0").redirect(Policy::limited(5)).build().unwrap();
        let resp = client.get(update_url).send().unwrap();
        write("pc_manager.msi", resp.bytes().unwrap()).unwrap();
        return Ok(());
    }

    pub fn install_update_win(
        update_url: String,
    ) -> Result<(), ()> {
        let prevdir = current_dir().unwrap();
        let tmpdir = tempdir::TempDir::new("pc_manager").unwrap();
        set_current_dir(tmpdir.path()).unwrap();

        download_package(update_url).unwrap();
        let command = format!("{}\\pc_manager.msi", tmpdir.path().display());
        Command::new("msiexec").args(["/i", command.as_str(), "/passive"]).output().unwrap();

        set_current_dir(prevdir).unwrap();
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
                    let asset_url = asset.get("browser_download_url").unwrap().as_str().unwrap();
                    return Ok(String::from(asset_url));
                }
            }
        }

        return Err(String::from("Failed to find release URL"));
    }

    #[cfg(test)]
    mod tests {
        use std::env::{current_dir, set_current_dir};
        use std::fs::{read_dir, remove_file};
        use regex::Regex;
        use crate::tasks::update::update::{download_package, get_update_url, task_update};

        #[test]
        #[ignore]
        fn test_task_update() {
            task_update();
        }

        #[test]
        #[ignore]
        fn test_download_package() {
            let prevdir = current_dir().unwrap();
            let tmpdir = tempdir::TempDir::new("pc_manager").unwrap();
            set_current_dir(tmpdir.path()).unwrap();

            let update_url = get_update_url().unwrap();
            let res = download_package(update_url);
            assert_eq!(true, res.is_ok());

            let mut found = false;
            let paths = read_dir("./").unwrap();
            for path in paths {
                let path_str = path.unwrap().path().display().to_string();
                println!("File: {path_str}");
                if path_str.ends_with(".msi") {
                    remove_file(path_str).unwrap();
                    found = true;
                }
            }
            assert_eq!(true, found);

            set_current_dir(prevdir).unwrap();
        }

        #[test]
        fn test_get_update_url() {
            let result = get_update_url();
            assert_eq!(true, result.is_ok());
            let result_url = result.unwrap();
            println!("Update URL: {result_url}");
            let re = Regex::new(r"https://github\.com/cameroncros/pc_manager/releases/download/.*\.msi").unwrap();
            assert_eq!(true, re.is_match(result_url.as_str()));
        }
    }
}

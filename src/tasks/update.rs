#[cfg(target_os = "linux")]
pub(crate) mod update {
    use std::fs::{File, Permissions, set_permissions};
    use std::io::Write;
    use std::os::unix::fs::PermissionsExt;
    use std::process::{Command, exit};

    use nix::sys::wait::waitpid;
    use nix::unistd::{chdir, fork, ForkResult, setuid, User};
    use nix::unistd::seteuid;

    use crate::sensors::update::update::check_for_update;

    pub fn build_package(
        update_url: String,
    ) -> Result<(), ()> {
        let user = User::from_name("nobody").unwrap().unwrap();
        setuid(user.uid).unwrap();
        seteuid(user.uid).unwrap();
        let resp = reqwest::blocking::get(update_url).unwrap();
        {
            let mut file = File::create("PKGBUILD").unwrap();
            file.write_all(resp.bytes().unwrap().as_ref()).unwrap();
        }
        Command::new("makepkg");
        return Ok(());
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
        Command::new("pacman -U --noconfirm *.tar*");
        Command::new("systemctl restart pc_manager");
        return Ok(());
    }

    pub fn task_update() {
        let update_url = check_for_update().unwrap();
        install_update_arch(update_url).unwrap();
    }
}

#[cfg(target_os = "windows")]
pub(crate) mod update {
    pub fn task_update() {
        // TODO;
    }
}

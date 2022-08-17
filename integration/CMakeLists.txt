configure_file(docker-compose.yml docker-compose.yml COPYONLY)
configure_file(Dockerfile Dockerfile COPYONLY)
add_custom_target(integration
        COMMAND
            cp ../src/pc_manager ./ &&
            docker build . -t pc_manager_machine &&
            docker-compose up -d
        DEPENDS pc_manager)
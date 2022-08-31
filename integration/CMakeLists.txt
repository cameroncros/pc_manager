if (UNIX)
    file(COPY docker-compose.yml DESTINATION .)
    file(COPY Dockerfile DESTINATION .)
    file(COPY test_integration.py DESTINATION .)

    # Build command line to run py.test.
    set(PYTEST ${CMAKE_CURRENT_BINARY_DIR}/venv/bin/python3 -m pytest)

    add_test(
            NAME integration
            COMMAND
                ${PYTEST}
            DEPENDS
                pc_manager
    )

    add_custom_target(
            integration_setup ALL
            COMMAND
            cp ../src/pc_manager ./ &&
            docker build . -t pc_manager_machine
            DEPENDS pc_manager
    )

    # Find Python and Virtualenv. We don't actually use the output of the
    # find_package, but it'll give nicer errors.
    find_package(PythonInterp 3 REQUIRED)

    # Generate the virtualenv and ensure it's up to date.
    add_custom_target(
            venv_setup ALL
            COMMAND python3 -m venv ${CMAKE_CURRENT_BINARY_DIR}/venv
            COMMAND ${CMAKE_CURRENT_BINARY_DIR}/venv/bin/pip install -r ${CMAKE_CURRENT_SOURCE_DIR}/requirements.txt --no-binary=* --upgrade
    )
endif()
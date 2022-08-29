file(COPY docker-compose.yml DESTINATION .)
file(COPY Dockerfile DESTINATION .)
file(COPY test_integration.py DESTINATION .)

add_custom_target(
        integration_setup
        COMMAND
            cp ../src/pc_manager ./ &&
            docker build . -t pc_manager_machine &&
            docker-compose up -d
        DEPENDS pc_manager
)

# Find Python and Virtualenv. We don't actually use the output of the
# find_package, but it'll give nicer errors.
find_package(PythonInterp 3 REQUIRED)

# Generate the virtualenv and ensure it's up to date.
add_custom_command(
        OUTPUT venv
        COMMAND python3 -m venv venv
)
add_custom_command(
        OUTPUT venv.stamp
        DEPENDS venv requirements.txt
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/requirements.txt requirements.txt
        COMMAND ./venv/bin/pip install -r requirements.txt --upgrade
)

# Build command line to run py.test.
set(PYTEST
        ${CMAKE_CURRENT_BINARY_DIR}/venv/bin/python3 -m pytest
)

add_custom_target(Tests ALL
        DEPENDS venv.stamp
        SOURCES requirements.txt
)

add_test(
        NAME integration
        COMMAND
            ${PYTEST}
)
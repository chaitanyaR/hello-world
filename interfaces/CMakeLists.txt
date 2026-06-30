cmake_minimum_required(VERSION 3.22)

# When CommonAPI generator is available, add a custom target to regenerate
# proxy/stub code from .fidl + .fdepl files into interfaces/generated/.
# Until then, the generated/ directory holds hand-written stub headers.

if(CommonAPI_FOUND AND CommonAPI-SomeIP_FOUND)
    find_program(COMMONAPI_GENERATOR commonapi-generator REQUIRED)
    find_program(COMMONAPI_SOMEIP_GENERATOR commonapi-someip-generator REQUIRED)

    set(FIDL_DIR "${CMAKE_CURRENT_SOURCE_DIR}/service_definitions")
    set(FDEPL_DIR "${CMAKE_CURRENT_SOURCE_DIR}/deployment")
    set(GEN_DIR "${CMAKE_CURRENT_SOURCE_DIR}/generated")

    set(FIDL_FILES
        ${FIDL_DIR}/common_types.fidl
        ${FIDL_DIR}/hpc_powertrain.fidl
        ${FIDL_DIR}/hpc_adas.fidl
        ${FIDL_DIR}/hpc_telemetry.fidl
        ${FIDL_DIR}/hpc_body.fidl
        ${FIDL_DIR}/sovd_types.fidl
    )

    add_custom_target(generate_interfaces ALL
        COMMAND ${COMMONAPI_GENERATOR} -sk Default -d ${GEN_DIR} ${FIDL_FILES}
        COMMAND ${COMMONAPI_SOMEIP_GENERATOR} -d ${GEN_DIR}
                ${FDEPL_DIR}/hpc_powertrain_someip.fdepl
                ${FDEPL_DIR}/hpc_adas_someip.fdepl
                ${FDEPL_DIR}/hpc_telemetry_someip.fdepl
                ${FDEPL_DIR}/hpc_body_someip.fdepl
        DEPENDS ${FIDL_FILES}
        COMMENT "Generating CommonAPI proxy/stub code from Franca IDL"
    )
endif()

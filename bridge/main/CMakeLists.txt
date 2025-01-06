idf_component_register(
    SRCS
        "main.c"
        "udp_server.c"
        "usb_ncm.c"
        "enow.c"
    INCLUDE_DIRS "." ""
    PRIV_REQUIRES esp_wifi nvs_flash
    )

set(kio_rapip_SRCS
   rapip.cpp
)

kde4_add_plugin(kio_rapip ${kio_rapip_SRCS})
target_link_libraries(kio_rapip ${SYNCE_LDFLAGS} ${KDE4_KIO_LIBS})
install(TARGETS kio_rapip DESTINATION ${PLUGIN_INSTALL_DIR})

install( FILES synce.protocol rapip.protocol DESTINATION ${SERVICES_INSTALL_DIR} )

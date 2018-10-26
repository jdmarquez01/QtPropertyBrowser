# Tell CMake to run moc when necessary:
set(CMAKE_AUTOMOC ON)

# As moc files are generated in the binary dir, tell CMake
# to always look for includes there:
set(CMAKE_INCLUDE_CURRENT_DIR ON)
add_definitions(-DQT_QTPROPERTYBROWSER_EXPORT)
set(HEADERS qtpropertybrowser.h 
			qtpropertymanager.h 
			qteditorfactory.h 
			qtvariantproperty.h 
			qttreepropertybrowser.h 
			qtbuttonpropertybrowser.h 
			qtgroupboxpropertybrowser.h
			qtpropertybrowserutils_p.h
			qtmetaobjectpropertybrowser.h
			)


add_definitions(-DQT_DISABLE_DEPRECATED_BEFORE=0)
add_definitions(-DQT_QTPROPERTYBROWSER_EXPORT)
file(GLOB _PUBLIC_HDRS Qt*)
file(GLOB _IMPL_HDRS *.h)
set (SOURCES 
		qtpropertybrowser.cpp 
        qtpropertymanager.cpp 
        qteditorfactory.cpp 
        qtvariantproperty.cpp 
        qttreepropertybrowser.cpp 
        qtbuttonpropertybrowser.cpp 
        qtgroupboxpropertybrowser.cpp 
        qtpropertybrowserutils.cpp
		qtmetaobjectpropertybrowser.cpp
		)

            
set(RESOURCES qtpropertybrowser.qrc)

QT5_WRAP_UI(_UI_SRCS ${_UI_FORMS})
QT5_ADD_RESOURCES(_QRC_SRCS ${_RESOURCES})

set(TARGET_NAME ${PROJECT_NAME})

add_library( ${TARGET_NAME} ${SOURCES} ${HEADERS} ${UIS} ${RESOURCES})


target_link_libraries(${TARGET_NAME} Qt5::Widgets)

######################### Installation Stuff ###########################
include(GenerateExportHeader)
generate_export_header(${TARGET_NAME})

install(TARGETS ${TARGET_NAME}
  EXPORT ${TARGET_NAME}Targets
  RUNTIME DESTINATION ${INSTALL_BIN_DIR}
  LIBRARY DESTINATION ${INSTALL_LIB_DIR}
  ARCHIVE DESTINATION ${INSTALL_LIB_DIR}
  INCLUDES DESTINATION ${INSTALL_INCLUDE_DIR}
)


file(GLOB _PUBLIC_HDRS Qt*)
file(GLOB _IMPL_HDRS *.h)
install(
  FILES
    ${_PUBLIC_HDRS}
    ${_IMPL_HDRS}
  DESTINATION
    ${INSTALL_INCLUDE_DIR}
  COMPONENT
    Devel
)


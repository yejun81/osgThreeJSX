IF(DYNAMIC_OSGTHREEJSX)
    ADD_DEFINITIONS(-DOSGTHREEJSX_LIBRARY)
ELSE()
    ADD_DEFINITIONS(-DOSGTHREEJSX_LIBRARY_STATIC)
ENDIF()

SET(LIB_NAME osgThreeJSX)
SET(HEADER_PATH ${osgThreeJSX_SOURCE_DIR}/include/${LIB_NAME})
SET(TARGET_H
    ${HEADER_PATH}/Light
    ${HEADER_PATH}/AmbientLight
    ${HEADER_PATH}/DirectionalLight
    ${HEADER_PATH}/HemisphereLight
    ${HEADER_PATH}/InstanceGeometry
    ${HEADER_PATH}/PointLight
    ${HEADER_PATH}/ProbeLight
    ${HEADER_PATH}/RectAreaLight
    ${HEADER_PATH}/SpotLight
    ${HEADER_PATH}/ProbeLight
    ${HEADER_PATH}/MaterialNode
    ${HEADER_PATH}/Material
    ${HEADER_PATH}/Materials
    ${HEADER_PATH}/MaterialData
    ${HEADER_PATH}/Programs
    ${HEADER_PATH}/RenderState
    ${HEADER_PATH}/ShaderLib
    ${HEADER_PATH}/Export
    ${HEADER_PATH}/Animation
    ${HEADER_PATH}/Shadow
)

# FIXME: For OS X, need flag for Framework or dylib
SET(TARGET_SRC
    Light.cpp
    AmbientLight.cpp
    DirectionalLight.cpp
    HemisphereLight.cpp
    InstanceGeometry.cpp
    PointLight.cpp
    ProbeLight.cpp
    RectAreaLight.cpp
    SpotLight.cpp
    MaterialNode.cpp
    Material.cpp
    Materials.cpp
    MaterialData.cpp
    Programs.cpp
    RenderState.cpp
    ShaderLib.cpp
    Animation.cpp
	Shadow.cpp
    ${OSGTHREEJSX_VERSIONINFO_RC}
)

INCLUDE_DIRECTORIES(${OSG_INCLUDE_DIR})

SET(TARGET_LIBRARIES_VARS OSG_LIBRARY OSGDB_LIBRARY OSGUTIL_LIBRARY OPENTHREADS_LIBRARY OSGANIMATION_LIBRARY)

SET(COMPONENT_PKGCONFIG_DESCRIPTION "ThreeJS library for OpenSceneGraph")

SETUP_LIBRARY(${LIB_NAME})


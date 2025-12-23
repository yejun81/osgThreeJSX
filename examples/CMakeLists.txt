#######################################################
# this are setting used in SETUP_EXAMPLE macro
#######################################################
SET(TARGET_DEFAULT_PREFIX "example_")
SET(TARGET_DEFAULT_LABEL_PREFIX "Examples")

SET(TARGET_COMMON_LIBRARIES
    osgThreeJSX
)

IF(ANDROID)
    # ADD_SUBDIRECTORY(osgandroidsimple)
ELSE(ANDROID)

IF(DYNAMIC_OSGTHREEJSX)

    ADD_SUBDIRECTORY(Animation)
    ADD_SUBDIRECTORY(BasicMaterial)
    ADD_SUBDIRECTORY(GltfViewer)
    ADD_SUBDIRECTORY(Instance)
    ADD_SUBDIRECTORY(RectAreaLight)
    ADD_SUBDIRECTORY(Shadow)
    ADD_SUBDIRECTORY(ShadowVsm)
    ADD_SUBDIRECTORY(SpotLight)

    #to add subject to find socket#ADD_SUBDIRECTORY(osgcluster)

ELSE(DYNAMIC_OSGTHREEJSX)
    #needed on win32 or the linker get confused by _declspec declarations
    ADD_DEFINITIONS(-DOSGTHREEJSX_LIBRARY_STATIC)

ENDIF(DYNAMIC_OSGTHREEJSX)

ENDIF(ANDROID)

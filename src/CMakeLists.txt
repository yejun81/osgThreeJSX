IF(ANDROID)
    SET(MODULE_USER_STATIC_OR_DYNAMIC ${OSGTHREEJSX_USER_DEFINED_DYNAMIC_OR_STATIC})
ENDIF(ANDROID)


#the old construct SUBDIRS( was substituted by ADD_SUBDIRECTORY that is to be preferred according on CMake docs.
FOREACH( mylibfolder
        osgThreeJSX
    )

    ADD_SUBDIRECTORY(${mylibfolder})

ENDFOREACH()

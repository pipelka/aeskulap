# declare directories containing used libraries
LINK_DIRECTORIES(${imagectn_BINARY_DIR} ${ofstd_BINARY_DIR} ${dcmdata_BINARY_DIR} ${dcmnet_BINARY_DIR})
IF(WITH_ZLIB)
  LINK_DIRECTORIES(${WITH_ZLIBINC}/lib)
ENDIF(WITH_ZLIB)

# declare executables
ADD_EXECUTABLE(imagectn cnf cnfexnt cnfpriv imagectn sce sceecho scefind sceget scemove scestore)
ADD_EXECUTABLE(ti cnf cnfexnt cnfpriv ti tinet tiquery tiui)
ADD_EXECUTABLE(dbregimg dbregimg)

# make sure executables are linked to the corresponding libraries
# imagectn
IF(WITH_ZLIB)
  TARGET_LINK_LIBRARIES(imagectn netapi32 wsock32 ofstd dcmdata dcmnet imagedb debug zlib_d optimized zlib_o)
ELSE(WITH_ZLIB)
  TARGET_LINK_LIBRARIES(imagectn netapi32 wsock32 ofstd dcmdata dcmnet imagedb)
ENDIF(WITH_ZLIB)
# ti
IF(WITH_ZLIB)
  TARGET_LINK_LIBRARIES(ti netapi32 wsock32 ofstd dcmdata dcmnet imagedb debug zlib_d optimized zlib_o)
ELSE(WITH_ZLIB)
  TARGET_LINK_LIBRARIES(ti netapi32 wsock32 ofstd dcmdata dcmnet imagedb)
ENDIF(WITH_ZLIB)
# dbregimg
IF(WITH_ZLIB)
  TARGET_LINK_LIBRARIES(dbregimg netapi32 wsock32 ofstd dcmdata dcmnet imagedb debug zlib_d optimized zlib_o)
ELSE(WITH_ZLIB)
  TARGET_LINK_LIBRARIES(dbregimg netapi32 wsock32 ofstd dcmdata dcmnet imagedb)
ENDIF(WITH_ZLIB)

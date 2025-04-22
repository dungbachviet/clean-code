CONFIG += c++11

*-g++ {
    QMAKE_CXXFLAGS += -Wno-unused-parameter
	QMAKE_CXXFLAGS += -fpermissive
	QMAKE_LIBS += -lbotan-2
	DEFINES += Q_OS_LINUX
}

win32-msvc* {
	QMAKE_CXXFLAGS_RELEASE += /Zi

	@
	QMAKE_LFLAGS_RELEASE += /DEBUG
	QMAKE_LFLAGS_RELEASE += /OPT:REF
	QMAKE_LFLAGS_RELEASE += /OPT:ICF
	@

    # Don't show warning for unused parameter
    W=""
    for (a, QMAKE_CXXFLAGS_WARN_ON):!equals(a, "-w34100"):W += $${a}
    QMAKE_CXXFLAGS_WARN_ON=$$W
}

DEFINES += PISCADA_SERVER

win32:DELETE = del
# Different delete commands for unix and windows shell
!win32:DELETE = rm -f

#win32 {
#    UNAME = $$system(uname -s)
#    FOUND+= $$find(UNAME, MINGW)
#    FOUND+= $$find(UNAME, MSYS)
#    count(FOUND, 0):DELETE = del
#}

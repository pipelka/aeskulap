# Microsoft Developer Studio Project File - Name="ALL_BUILD" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Generic Project" 0x010a

CFG=ALL_BUILD - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "ALL_BUILD.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "ALL_BUILD.mak" CFG="ALL_BUILD - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "ALL_BUILD - Win32 MinSizeRel" (based on "Win32 (x86) Generic Project")
!MESSAGE "ALL_BUILD - Win32 Release" (based on "Win32 (x86) Generic Project")
!MESSAGE "ALL_BUILD - Win32 RelWithDebInfo" (based on "Win32 (x86) Generic Project")
!MESSAGE "ALL_BUILD - Win32 Debug" (based on "Win32 (x86) Generic Project")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
MTL=midl.exe

!IF  "$(CFG)" == "ALL_BUILD - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""



!ELSEIF  "$(CFG)" == "ALL_BUILD - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""



!ELSEIF  "$(CFG)" == "ALL_BUILD - Win32 MinSizeRel"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "MinSizeRel"
# PROP BASE Intermediate_Dir "MinSizeRel"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "MinSizeRel"
# PROP Intermediate_Dir "MinSizeRel"
# PROP Target_Dir ""



!ELSEIF  "$(CFG)" == "ALL_BUILD - Win32 RelWithDebInfo"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "RelWithDebInfo"
# PROP BASE Intermediate_Dir "RelWithDebInfo"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "RelWithDebInfo"
# PROP Intermediate_Dir "RelWithDebInfo"
# PROP Target_Dir ""



!ENDIF 

# Begin Target

# Name "ALL_BUILD - Win32 Release"
# Name "ALL_BUILD - Win32 Debug"
# Name "ALL_BUILD - Win32 MinSizeRel"
# Name "ALL_BUILD - Win32 RelWithDebInfo"
# Begin Group "CMake Rules"
# PROP Default_Filter ""
# Begin Source File

SOURCE=ALL_BUILD_force_1.rule

!IF  "$(CFG)" == "ALL_BUILD - Win32 Release"
USERDEP__HACK=
# PROP Ignore_Default_Tool 1
# Begin Custom Build - Building Custom Rule $(InputPath)

ALL_BUILD_force_1 :  "$(SOURCE)" "$(INTDIR)" "$(OUTDIR)"
	echo "Build all projects" 


# End Custom Build

!ELSEIF  "$(CFG)" == "ALL_BUILD - Win32 Debug"
USERDEP__HACK=
# PROP Ignore_Default_Tool 1
# Begin Custom Build - Building Custom Rule $(InputPath)

ALL_BUILD_force_1 :  "$(SOURCE)" "$(INTDIR)" "$(OUTDIR)"
	echo "Build all projects" 


# End Custom Build

!ELSEIF  "$(CFG)" == "ALL_BUILD - Win32 MinSizeRel"
USERDEP__HACK=
# PROP Ignore_Default_Tool 1
# Begin Custom Build - Building Custom Rule $(InputPath)

ALL_BUILD_force_1 :  "$(SOURCE)" "$(INTDIR)" "$(OUTDIR)"
	echo "Build all projects" 


# End Custom Build

!ELSEIF  "$(CFG)" == "ALL_BUILD - Win32 RelWithDebInfo"
USERDEP__HACK=
# PROP Ignore_Default_Tool 1
# Begin Custom Build - Building Custom Rule $(InputPath)

ALL_BUILD_force_1 :  "$(SOURCE)" "$(INTDIR)" "$(OUTDIR)"
	echo "Build all projects" 


# End Custom Build

!ENDIF

# End Source File
# End Group
# End Target
# End Project

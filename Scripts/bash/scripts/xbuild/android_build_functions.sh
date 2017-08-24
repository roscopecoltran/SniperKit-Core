#!/bin/sh

# import common helper functions
. common_build_functions.sh

function UUClearCrashlyticsKeyAndSecret
{
	if [ $# != 1 ]
	then
		echo "Usage: UUClearCrashlyticsKeyAndSecret [Android manifest path]"
		exit 1
	fi
	
	local MANIFEST_PATH=$1
	
	# NOTE: This only works if you put the manifest entry on a single line.
    REPLACEMENT_VALUE="INSERT_FABRIC_API_KEY_HERE"
    OLD_LINE="(<meta-data android:name=\"io.fabric.ApiKey\" android:value=\")(.+)(\" \/>)"
    NEW_LINE="\1${REPLACEMENT_VALUE}\3"
    
	sed -i "" -E "s/${OLD_LINE}/${NEW_LINE}/g" "${MANIFEST_PATH}"
}

function UUExtractGradleBuildSetting
{
	if [ $# != 3 ]
	then
		echo "Usage: UUExtractGradleBuildSetting [Project Path] [VarName] [Output Variable]"
		exit 1
	fi
	
	local FULL_PATH=$1
	local VAR_NAME=$2
	local OUTPUT_VAR=$3
	
	local RESULT=`cat ${FULL_PATH} | grep -m 1 "${VAR_NAME}" | awk -F"${VAR_NAME}" '{print $2}' | sed 's/^ *//g' | sed 's/ *$//g' | tr -d '"'`
	
	eval $OUTPUT_VAR="'${RESULT}'"
	
	UUCheckReturnCode $? "UUExtractGradleBuildSetting"
}

function UUExtractGradleBuildProperty
{
	if [ $# != 3 ]
	then
		echo "Usage: UUExtractGradleBuildSetting [Project Path] [VarName] [Output Variable]"
		exit 1
	fi
	
	local FULL_PATH=$1
	local VAR_NAME=$2
	local OUTPUT_VAR=$3
	
	local RESULT=`cat ${FULL_PATH} | grep -m 1 "${VAR_NAME}=" | awk -F"${VAR_NAME}=" '{print $2}' | sed 's/^ *//g' | sed 's/ *$//g' | tr -d '"'`
	
	eval $OUTPUT_VAR="'${RESULT}'"
	
	UUCheckReturnCode $? "UUExtractGradleBuildSetting"
}

function UUMakeAarBuild
{
	if [ $# != 3 ]
	then
		echo "Usage: UUMakeAarBuild [Project Path] [Module] [Output Folder]"
		exit 1
	fi
	
	local PROJDIR=$1
	local MODULE=$2
	local OUTPUT_DIR=$3

	CWD=$(pwd)

	UUDebugLog "Changing current directory to ${PROJDIR}"
	cd "${PROJDIR}"

	./gradlew :${MODULE}:clean :${MODULE}:assembleRelease

	rm -rf ${OUTPUT_DIR}/${MODULE}.aar
	cp ${MODULE}/build/outputs/aar/${MODULE}-release.aar ${OUTPUT_DIR}/${MODULE}.aar

	rm -rf ${OUTPUT_DIR}/${MODULE}SymbolMapping.zip
	zip -r ${OUTPUT_DIR}/${MODULE}SymbolMapping.zip ${MODULE}/build/outputs/mapping

	UUCheckReturnCode $? "UUMakeAarBuild"
}

function UUMakeJavaDocBuild
{
	if [ $# != 3 ]
	then
		echo "Usage: UUMakeJavaDocBuild [Project Path] [Module] [Output Folder]"
		exit 1
	fi
	
	local PROJDIR=$1
	local MODULE=$2
	local OUTPUT_DIR=$3

	CWD=$(pwd)

	UUDebugLog "Changing current directory to ${PROJDIR}"
	cd "${PROJDIR}"

	./gradlew :${MODULE}:clean :${MODULE}:javadoc

	rm -rf ${OUTPUT_DIR}/${MODULE}Docs.zip
	
	cd ${MODULE}/build/docs/javadoc
	zip -r ${OUTPUT_DIR}/${MODULE}Docs.zip *

	UUCheckReturnCode $? "UUMakeJavaDocBuild"
}

function UUMakeApkBuild
{
	if [ $# != 6 ]
	then
		echo "Usage: UUMakeApkBuild [Project Path] [Module] [Flavor] [Output Folder] [Fixed Version] [APK Name]"
		exit 1
	fi
	
	local PROJDIR=$1
	local MODULE=$2
	local FLAVOR=$3
	local OUTPUT_DIR=$4
	local BUILD_VERSION=$5
	local APK_NAME=$6
	
	local UPLOAD_TO_CRASHLYTICS=0
	local HAS_FIXED_VERSION=0
	
	if [ ! -z ${BUILD_VERSION} ]
	then
		HAS_FIXED_VERSION=1
	fi
	
	if [ -z ${APK_NAME} ]
	then
		APK_NAME="${MODULE}"
	fi

	CWD=$(pwd)
	
	UUIsGitRepo IS_GIT_REPO
	UUIsSvnRepo IS_SVN_REPO
	
	GRADLE_PATH=${PROJDIR}/${MODULE}/build.gradle
	GRADLE_PROPERTIES_PATH=${PROJDIR}/gradle.properties
	
	UUExtractGradleBuildSetting "${GRADLE_PATH}" "ext.betaDistributionGroupAliases" CRASHLYTICS_GROUP_ALIAS
	
	if [ ! -z ${CRASHLYTICS_GROUP_ALIAS} ]
	then
		UPLOAD_TO_CRASHLYTICS=1
	fi

	UUDebugLog "PROJDIR: ${PROJDIR}"
	UUDebugLog "MODULE: ${MODULE}"
	UUDebugLog "OUTPUT_DIR: ${OUTPUT_DIR}"	
	UUDebugLog "UPLOAD_TO_CRASHLYTICS: ${UPLOAD_TO_CRASHLYTICS}"
	UUDebugLog "CRASHLYTICS_GROUP_ALIAS: ${CRASHLYTICS_GROUP_ALIAS}"
	UUDebugLog "BUILD_VERSION: ${BUILD_VERSION}"
	UUDebugLog "HAS_FIXED_VERSION: ${HAS_FIXED_VERSION}"
	UUDebugLog "IS_GIT_REPO: ${IS_GIT_REPO}"
	UUDebugLog "IS_SVN_REPO: ${IS_SVN_REPO}"

	VERSION_NAME_VAR=buildVersionName
	
	if [ ${HAS_FIXED_VERSION} == 0 ]
	then
		UUExtractGradleBuildProperty "${GRADLE_PROPERTIES_PATH}" "${VERSION_NAME_VAR}" CURRENT_VERSION
		BUILD_VERSION=${CURRENT_VERSION}
		
		if [ ${IS_GIT_REPO} == 1 ]
		then
			UUReadGitRevisionNumber GIT_REV_NUMBER
			BUILD_VERSION="${CURRENT_VERSION}.${GIT_REV_NUMBER}"
		fi
		
			UUDebugLog "CURRENT_VERSION=${CURRENT_VERSION}"
			UUDebugLog "GIT_REV_NUMBER=${GIT_REV_NUMBER}"
			UUDebugLog "BUILD_VERSION=${BUILD_VERSION}"
	fi
	
	UUDebugLog "Changing current directory to ${PROJDIR}"
	cd "${PROJDIR}"

	if [ ${UPLOAD_TO_CRASHLYTICS} == 1 ]
	then
		UUDebugLog "Building with Crashlytics Upload"
		./gradlew :${MODULE}:clean :${MODULE}:assemble${FLAVOR}Release -P${VERSION_NAME_VAR}="${BUILD_VERSION}" crashlyticsUploadDistribution${FLAVOR}Release
	else
		UUDebugLog "Building without Crashlytics Upload"
		./gradlew :${MODULE}:clean :${MODULE}:assemble${FLAVOR}Release -P${VERSION_NAME_VAR}="${BUILD_VERSION}"
	fi
	
	UUCheckReturnCode $? "UUMakeApkBuild.gradle compile"
	
	BUILT_APK_NAME=${MODULE}-${FLAVOR}-release.apk
	if [ -z ${FLAVOR} ]
	then
		BUILT_APK_NAME=${MODULE}-release.apk
	fi
	
	BUILT_APK_FULL_PATH=${MODULE}/build/outputs/apk/${BUILT_APK_NAME}
	OUTPUT_APK_NAME=${APK_NAME}-${BUILD_VERSION}.apk
	UUDebugLog "BUILT_APK_NAME: ${BUILT_APK_NAME}"
	UUDebugLog "BUILT_APK_FULL_PATH: ${BUILT_APK_FULL_PATH}"
	UUDebugLog "OUTPUT_APK_NAME: ${OUTPUT_APK_NAME}"
	
	rm -rf ${OUTPUT_DIR}/${OUTPUT_APK_NAME}
	
	UUDebugLog "Copying APK to output folder"
	cp ${BUILT_APK_FULL_PATH} ${OUTPUT_DIR}/${OUTPUT_APK_NAME}

	UUCheckReturnCode $? "UUMakeApkBuild"
}
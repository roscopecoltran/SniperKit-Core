#!/bin/sh

# import common helper functions
. common_build_functions.sh

function UUSetupBuildKeychain
{
	if [ $# -lt 4 ]
	then
		echo "Usage: UUSetupBuildKeychain [Keychain Path] [Keychain Password] [Cert Path] [Cert Password] (optional) [Verbose Mode]"
		exit 1
	fi
	
	local KEYCHAIN_PATH=$1
	local KEYCHAIN_PASS=$2
	local IMPORT_CERT_PATH=$3
	local IMPORT_CERT_PASS=$4
	local VERBOSE_MODE=$5
	
	UUDebugLog "Creating temporary build keychain ${KEYCHAIN_PATH}"
	security ${VERBOSE_MODE} create-keychain -p ${KEYCHAIN_PASS} ${KEYCHAIN_PATH}
	if [ $? == 48 ] # keychain already exists
	then
		UUDebugLog "Keychain already exists, let's try deleting it"
		security ${VERBOSE_MODE} delete-keychain ${KEYCHAIN_PATH}
		
		UUDebugLog "Trying to creating keychain again"
		security ${VERBOSE_MODE} create-keychain -p ${KEYCHAIN_PASS} ${KEYCHAIN_PATH}
	fi
	
	UUCheckReturnCode $? "createKeychain"
	
	UUDebugLog "Unlocking temporary build keychain ${KEYCHAIN_PATH}"
	security ${VERBOSE_MODE} unlock-keychain -p ${KEYCHAIN_PASS} ${KEYCHAIN_PATH}
	UUCheckReturnCode $? "unlockKeychain"
		
	UUDebugLog "Importing build certificate into keychain"
	security ${VERBOSE_MODE} import ${IMPORT_CERT_PATH} -k ${KEYCHAIN_PATH} -P ${IMPORT_CERT_PASS} -T /usr/bin/codesign -A
	UUCheckReturnCode $? "importCertificate"

	UUDebugLog "Listing keychain"
	security ${VERBOSE_MODE} list-keychain -s ${KEYCHAIN_PATH}
	UUCheckReturnCode $? "listKeychain"

	UUDebugLog "Setting keychain timeout"
	security -v set-keychain-settings -lut 7200 ${KEYCHAIN_PATH}
	UUCheckReturnCode $? "setKeychainTimeout"
	
	UUDebugLog "Setting key paritition list ${KEYCHAIN_PATH}"
	security ${VERBOSE_MODE} set-key-partition-list -S apple-tool:,apple: -s -k ${KEYCHAIN_PASS} ${KEYCHAIN_PATH}
	
	# Don't check return code here because this is a Sierra only command
	# UUCheckReturnCode $? "setKeyPartition List"
}

function UUCleanupBuildKeychain
{
	if [ $# -lt 1 ]
	then
		echo "Usage: UUCleanupBuildKeychain [Keychain Path] (optional) [Verbose Mode]"
		exit 1
	fi
	
	local KEYCHAIN_PATH=$1
	local VERBOSE_MODE=$2
	
	UUDebugLog "Deleting temporary keychain ${KEYCHAIN_PATH}"
	security ${VERBOSE_MODE} delete-keychain ${KEYCHAIN_PATH}
	UUCheckReturnCode $? "deleteKeychain"

    UUDebugLog "Re-listing current user login keychain"
    security ${VERBOSE_MODE} list-keychain -s  "${HOME}/Library/Keychains/login.keychain"
}

function UUMakeTempKeychain
{
	if [ $# -lt 3 ]
	then
		echo "Usage: UUMakeTempKeychain [path to certificate] [certificate password] [keychain path (out)]"
		exit 1
	fi

	CWD=$(pwd)

	local CERT_PATH=$1
	local CERT_PASSWORD=$2
	local OUTPUT_VAR=$3
	
	KEYCHAIN_NAME=`uuidgen`
	KEYCHAIN_PATH=~/Library/Keychains/${KEYCHAIN_NAME}.keychain
	KEYCHAIN_PASS=masterpassword
	UUDebugLog "KEYCHAIN_NAME: ${KEYCHAIN_NAME}"
	
	UUSetupBuildKeychain ${KEYCHAIN_PATH} ${KEYCHAIN_PASS} ${CERT_PATH} ${CERT_PASSWORD} -v
	
	eval $OUTPUT_VAR="'${KEYCHAIN_PATH}'"
}

function UUReadPlistString
{
	if [ $# != 3 ]
	then
		echo "Usage: UUReadPlistString [Full Path to Plist] [VarName] [Output Variable]"
		exit 1
	fi
	
	local PLIST_PATH=$1
	local VAR_NAME=$2
	local OUTPUT_VAR=$3
	
	VAR_RESULT=`/usr/libexec/PlistBuddy -c "Print :${VAR_NAME}" "${PLIST_PATH}"`
    eval $OUTPUT_VAR="'${VAR_RESULT}'"
    
    UUCheckReturnCode $? "read ${VAR_NAME}"
}

function UUReadBundleVersion
{
	UUReadPlistString "$1" "CFBundleVersion" "$2"
}

function UUReadBundleShortVersionString
{
	UUReadPlistString "$1" "CFBundleShortVersionString" "$2"
}

function UUWritePlistString
{
	if [ $# != 3 ]
	then
		echo "Usage: UUWritePlistString [Full Path to Plist] [VarName] [VarValue]"
		exit 1
	fi
	
	local PLIST_PATH=$1
	local VAR_NAME=$2
	local VAR_VALUE=$3
	
	`/usr/libexec/PlistBuddy -c "Set :${VAR_NAME} ${VAR_VALUE}" "${PLIST_PATH}"`
	
    UUCheckReturnCode $? "write ${VAR_NAME}"
}

function UUUpdateWatchAppBundleIdentifier
{
	if [ $# != 2 ]
    then
        echo "Usage: UUUpdateWatchAppBunbleIdentifier [Plist Path] [bundle id]"
        exit 1
    fi
    
    local PLIST_PATH=$1
    local BUNDLE_ID=$2
    
    UUDebugLog "PLIST_PATH: ${PLIST_PATH}"
    UUDebugLog "BUNDLE_ID: ${BUNDLE_ID}"
    
    `/usr/libexec/PlistBuddy -c "Set :NSExtension:NSExtensionAttributes:WKAppBundleIdentifier ${BUNDLE_ID}" "${PLIST_PATH}"`
	
    UUCheckReturnCode $? "write watch bundle id ${VAR_NAME}"
}

function UUWriteBundleVersion
{
	UUWritePlistString "$1" "CFBundleVersion" "$2"
}

function UUWriteBundleShortVersionString
{
	UUWritePlistString "$1" "CFBundleShortVersionString" "$2"
}

function UUWriteBundleVersionShort
{
	local VERSION=$2
	UUGetShortBuildNumber ${VERSION} UPDATED_VERSION
	
	UUDebugLog "VERSION: ${VERSION}, UPDATED_VERSION: ${UPDATED_VERSION}"

	UUWritePlistString "$1" "CFBundleShortVersionString" "${UPDATED_VERSION}"
}

function UUWriteBundleIdentifier
{
	UUWritePlistString "$1" "CFBundleIdentifier" "$2"
}

function UUWriteBundleDisplayName
{
	UUWritePlistString "$1" "CFBundleDisplayName" "$2"
}

function UUGenerateArchive
{
	if [ $# != 4 ]
	then
		echo "Usage: UUGenerateArchive [full project path] [scheme name] [configuration (Release/Debug)] [Archive Output Path]"
		exit 1
	fi
	
	local PROJECT_PATH=$1
	local SCHEME=$2
	local CONFIGURATION=$3
	local ARCHIVE_OUTPUT_PATH=$4
	
	UUDebugLog "UUGenerateArchive, PROJECT_PATH=${PROJECT_PATH}"
	UUDebugLog "UUGenerateArchive, SCHEME=${SCHEME}"
	UUDebugLog "UUGenerateArchive, CONFIGURATION=${CONFIGURATION}"
	UUDebugLog "UUGenerateArchive, ARCHIVE_OUTPUT_PATH=${ARCHIVE_OUTPUT_PATH}"
	
	BUILD_TYPE="-project"
	
	if [[ ${PROJECT_PATH} == *".xcworkspace" ]]
	then
		BUILD_TYPE="-workspace"
	fi
	
	UUDebugLog "Building Xcode Project"
	xcodebuild \
		"${BUILD_TYPE}" "${PROJECT_PATH}" \
		-scheme "${SCHEME}" \
		-configuration "${CONFIGURATION}" \
		-archivePath "${ARCHIVE_OUTPUT_PATH}" \
		clean \
		archive
	
	UUCheckReturnCode $? "Xcode Build Failed"
}

function UUGenerateBuild
{
    if [ $# != 5 ]
    then
        echo "Usage: UUGenerateBuild [full project path] [scheme name] [sdk] [configuration (Release/Debug)] [Output Path]"
    exit 1
    fi

    local PROJECT_PATH=$1
    local SCHEME=$2
    local SDK=$3
    local CONFIGURATION=$4
    local OUTPUTDIR=$5

    UUExtractXcodeBuildSetting "${PROJECT_PATH}" "${CONFIGURATION}" "ENABLE_BITCODE" ENABLE_BITCODE
    UUExtractXcodeBuildSetting "${PROJECT_PATH}" "${CONFIGURATION}" "OTHER_CFLAGS" OTHER_CFLAGS

	UUDebugLog "ENABLE_BITCODE: ${ENABLE_BITCODE}"
	UUDebugLog "OTHER_CFLAGS: ${OTHER_CFLAGS}"
	
    BUILD_TYPE="-project"

    if [[ ${PROJECT_PATH} == *".xcworkspace" ]]
    then
        BUILD_TYPE="-workspace"
    fi
    
    if [[ ${ENABLE_BITCODE} == "YES" ]]
    then
    	UUDebugLog "Bitcode is enabled, force c flags"
    	OTHER_CFLAGS="${OTHER_CFLAGS} -fembed-bitcode -Qunused-arguments"
    fi
    
    if [[ ${SDK} == "iphonesimulator" ]]
    then
    
    	UUDebugLog "Building Xcode Project for simulator platform"
		xcodebuild \
			"${BUILD_TYPE}" "${PROJECT_PATH}" \
			-scheme "${SCHEME}" \
			-sdk "${SDK}" \
			-configuration "${CONFIGURATION}" \
			OTHER_CFLAGS="${OTHER_CFLAGS}" \
			CONFIGURATION_BUILD_DIR="${OUTPUTDIR}" \
			-destination 'platform=iOS Simulator,name=iPhone' \
			clean \
			build
	else
	
	    UUDebugLog "Building Xcode Project for iPhone platform"
		xcodebuild \
			"${BUILD_TYPE}" "${PROJECT_PATH}" \
			-scheme "${SCHEME}" \
			-sdk "${SDK}" \
			-configuration "${CONFIGURATION}" \
			OTHER_CFLAGS="${OTHER_CFLAGS}" \
			CONFIGURATION_BUILD_DIR="${OUTPUTDIR}" \
			clean \
			build

    fi

    UUCheckReturnCode $? "Xcode Build Failed"
}

function UUGenerateIpa
{
	if [ $# != 4 ]
	then
		echo "Usage: UUGenerateIpa [archive path] [ipa output path] [team identifer] [export type]"
		exit 1
	fi

	local ARCHIVE_PATH=$1
	local IPA_OUTPUT_PATH=$2
	local TEAM_IDENTIFIER=$3
	local EXPORT_TYPE=$4
	
	EXPORT_PLIST_FILE=/tmp/__${TEAM_IDENTIFIER}__.plist
	rm -rf "${EXPORT_PLIST_FILE}"
	
	cat > "${EXPORT_PLIST_FILE}" <<EOF
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
   <key>method</key>
   <string>${EXPORT_TYPE}</string>
   <key>teamID</key>
   <string>${TEAM_IDENTIFIER}</string>
   <key>uploadSymbols</key>
   <string>false</string>
</dict>
</plist>
EOF

	xcodebuild \
		-exportArchive \
		-archivePath "${ARCHIVE_PATH}" \
		-exportOptionsPlist "${EXPORT_PLIST_FILE}" \
		-exportPath "${IPA_OUTPUT_PATH}"

	rm -rf "${EXPORT_PLIST_FILE}"

	UUCheckReturnCode $? "Generate IPA"
}

function UUUploadToCrashlytics
{
	if [ $# != 6 ]
	then
		echo "Usage: UUUploadToCrashlytics [api key] [build secret] [crashlytics path] [email group aliases] [notes path] [ipa path]"
		exit 1
	fi
	
	local API_KEY=$1
	local BUILD_SECRET=$2
	local CRASHLYTICS_PATH=$3
	local EMAIL_LIST=$4
	local NOTES_PATH=$5
	local IPA_PATH=$6
	
	local SUBMIT_EXE_PATH=${CRASHLYTICS_PATH}/submit
	if [ -f ${SUBMIT_EXE_PATH} ]
	then
	
		${SUBMIT_EXE_PATH} \
			${API_KEY} ${CRASHLYTICS_BUILD_SECRET} \
			-ipaPath "${IPA_PATH}" \
			-groupAliases "${EMAIL_LIST}" \
			-notesPath "${NOTES_PATH}" \
			-notifications YES

		UUCheckReturnCode $? "Crashlytics Upload"
	
	else
	
		UUDebugLog "Crashlytics submit exe not found, skipping upload"
	
	fi
}

function UUExtractXcodeBuildSetting
{
	if [ $# != 4 ]
	then
		echo "Usage: UUExtractXcodeBuildSetting [Project Path] [config name] [VarName] [Output Variable]"
		exit 1
	fi
	
	local FULL_PATH=$1
	local CONFIG_NAME=$2
	local VAR_NAME=$3
	local OUTPUT_VAR=$4
	
	BUILD_TYPE="-project"
	if [[ ${FULL_PATH} == *".xcworkspace" ]]
	then
		BUILD_TYPE="-workspace"
	fi
	
	local RESULT=$(xcodebuild "${BUILD_TYPE}" "${FULL_PATH}" -configuration "${CONFIG_NAME}" -showBuildSettings | grep -m 1 ${VAR_NAME} | awk -F"=" '/=/ { print $2 }' | sed 's/^ *//g' | sed 's/ *$//g') 
	
	eval $OUTPUT_VAR="'${RESULT}'"
	
	UUCheckReturnCode $? "UUExtractXcodeBuildSetting"
}

function UUExtractMobileProvisionValue
{
	if [ $# != 3 ]
	then
		echo "Usage: UUExtractMobileProvisionValue [Provision Path] [VarName] [Output Variable]"
		exit 1
	fi
	
	local FULL_PATH=$1
	local VAR_NAME=$2
	local OUTPUT_VAR=$3
	
	local TMP_PLIST_PATH=tmp.plist
	
	local RESULT=`security cms -D -i "${FULL_PATH}"  > "${TMP_PLIST_PATH}" && /usr/libexec/PlistBuddy -c "Print ${VAR_NAME}" "${TMP_PLIST_PATH}"`
	
	eval $OUTPUT_VAR="'${RESULT}'"
	
	UUCheckReturnCode $? "UUExtractMobileProvisionValue"

	rm -rf "${TMP_PLIST_PATH}"
}

function UUExtractCrashlyticsKeyAndSecret
{
	if [ $# != 3 ]
	then
		echo "Usage: UUExtractCrashlyticsKeyAndSecret [Xcode project path] [API Key output variable] [Build Secret Output Variable]"
		exit 1
	fi
	
	local PROJECT_PATH=$1
	local API_KEY_OUT=$2
	local BUILD_SECRET_OUT=$3
	
	local PBXPROJECT_PATH="${PROJECT_PATH}/project.pbxproj"
	
	TMP=`cat "${PBXPROJECT_PATH}" | grep -m 1 "Fabric.framework/run" | awk -F"=" '/=/ { print $2 }'`
	UUDebugLog "TMP: ${TMP}"

	if [ -z "${TMP}" ]
	then
		UUDebugLog "Checking Pods style script phase"
		TMP=`cat "${PBXPROJECT_PATH}" | grep -m 1 "${PODS_ROOT}/Fabric/run" | awk -F"=" '/=/ { print $2 }'`
		UUDebugLog "TMP: ${TMP}"
	fi
	
	if [ -z "${TMP}" ]
	then
		UUDebugLog "Checking old crashlytics style"
		TMP=`cat "${PBXPROJECT_PATH}" | grep -m 1 "Crashlytics.framework/run" | awk -F"=" '/=/ { print $2 }'`
		UUDebugLog "TMP: ${TMP}"
	fi
	
	TMP="${TMP/\"/}"
	TMP="${TMP/\";/}"
	UUDebugLog "TMP: ${TMP}"
	
	local API_KEY_RESULT=""
	local BUILD_SECRET_RESULT=""
	
	INDEX=0
	for word in ${TMP}
	do
		UUDebugLog "WORD: ${word}"
		
		if [ ${INDEX} = 1 ]
		then
			API_KEY_RESULT=${word}
		fi
		
		if [ ${INDEX} = 2 ]
		then
			BUILD_SECRET_RESULT=${word}
		fi
		
		let INDEX=INDEX+1
	done
	
	eval $API_KEY_OUT="'${API_KEY_RESULT}'"
	UUCheckReturnCode $? "UUFindFolder assign api key result"
	
	eval $BUILD_SECRET_OUT="'${BUILD_SECRET_RESULT}'"
	UUCheckReturnCode $? "UUFindFolder assign build secret result"
}

function UUClearCrashlyticsKeyAndSecret
{
	if [ $# != 2 ]
	then
		echo "Usage: UUClearCrashlyticsKeyAndSecret [Xcode project path] [Config Name]"
		exit 1
	fi
	
	local PROJECT_PATH=$1
	local CONFIG_NAME=$2
	local PBXPROJECT_PATH="${PROJECT_PATH}/project.pbxproj"

	UUExtractCrashlyticsKeyAndSecret "${PROJECT_PATH}" CRASHLYTICS_API_KEY CRASHLYTICS_BUILD_SECRET
	
	UUReplaceStringInFile "${PBXPROJECT_PATH}" "${CRASHLYTICS_API_KEY}" "INSERT_CRASHLYTICS_API_KEY_HERE"
	UUReplaceStringInFile "${PBXPROJECT_PATH}" "${CRASHLYTICS_BUILD_SECRET}" "INSERT_CRASHLYTICS_BUILD_SECRET_HERE"
	
	UUExtractXcodeBuildSetting "${PROJECT_PATH}" "${CONFIG_NAME}" "INFOPLIST_FILE" INFO_PLIST_FILE
	
	UUReplaceStringInFile "${INFO_PLIST_FILE}" "${CRASHLYTICS_API_KEY}" "INSERT_CRASHLYTICS_API_KEY_HERE"
	UUReplaceStringInFile "${INFO_PLIST_FILE}" "${CRASHLYTICS_BUILD_SECRET}" "INSERT_CRASHLYTICS_BUILD_SECRET_HERE"
}

function UUUpdateProvisioningProfile
{
    if [ $# != 4 ]
    then
        echo "Usage: UUUpdateProvisioningProfile [Xcode project path] [profile name full] [profile guid] [profile name]"
        exit 1
    fi

    local PROJECT_PATH=$1
    local PROFILE_NAME=$2
    local PROFILE_GUID=$3
    local PROFILE_SPECIFIER_NAME=$4

	UUDebugLog "PROFILE_NAME: ${PROFILE_NAME}"
    local PBXPROJECT_PATH="${PROJECT_PATH}/project.pbxproj"

	PROFILE_NAME=`UUEscapeChars "${PROFILE_NAME}"`
	UUDebugLog "PROFILE_NAME Escaped: ${PROFILE_NAME}"
	
	OLD_PROFILE_LINE="(\"CODE_SIGN_IDENTITY\[.+\]\" = \")(.+)(\";)"
	NEW_PROFILE_LINE="\1${PROFILE_NAME}\3"
	sed -i "" -E "s/${OLD_PROFILE_LINE}/${NEW_PROFILE_LINE}/g" "${PBXPROJECT_PATH}"
	
	OLD_PROFILE_LINE="(CODE_SIGN_IDENTITY = \")(.+)(\";)"
	sed -i "" -E "s/${OLD_PROFILE_LINE}/${NEW_PROFILE_LINE}/g" "${PBXPROJECT_PATH}"
	
	OLD_PROFILE_LINE="PROVISIONING_PROFILE = \"\";"
	NEW_PROFILE_LINE="PROVISIONING_PROFILE = \"${PROFILE_GUID}\";"
	sed -i "" -e "s/${OLD_PROFILE_LINE}/${NEW_PROFILE_LINE}/g" "${PBXPROJECT_PATH}"
	
	OLD_PROFILE_LINE="PROVISIONING_PROFILE_SPECIFIER = \"\";"
	NEW_PROFILE_LINE="PROVISIONING_PROFILE_SPECIFIER = \"${PROFILE_SPECIFIER_NAME}\";"
	sed -i "" -e "s/${OLD_PROFILE_LINE}/${NEW_PROFILE_LINE}/g" "${PBXPROJECT_PATH}"
}

function UUUpdateTeamIdentifier
{
    if [ $# != 2 ]
    then
        echo "Usage: UUUpdateTeamIdentifier [Xcode project path] [Team ID]"
        exit 1
    fi

    local PROJECT_PATH=$1
    local TEAM_NAME=$2

	UUDebugLog "TEAM_NAME: ${TEAM_NAME}"
    local PBXPROJECT_PATH="${PROJECT_PATH}/project.pbxproj"

	OLD_PROFILE_LINE="(DevelopmentTeam = )(.+)(;)"
	NEW_PROFILE_LINE="\1${TEAM_NAME}\3"
	UUDebugLog "OLD_PROFILE_LINE: ${OLD_PROFILE_LINE}"
	UUDebugLog "NEW_PROFILE_LINE: ${NEW_PROFILE_LINE}"
	
	sed -i "" -E "s/${OLD_PROFILE_LINE}/${NEW_PROFILE_LINE}/g" "${PBXPROJECT_PATH}"
	
	OLD_PROFILE_LINE="(DEVELOPMENT_TEAM = )(.+)(;)"
	NEW_PROFILE_LINE="\1${TEAM_NAME}\3"
	UUDebugLog "OLD_PROFILE_LINE: ${OLD_PROFILE_LINE}"
	UUDebugLog "NEW_PROFILE_LINE: ${NEW_PROFILE_LINE}"
	
	sed -i "" -E "s/${OLD_PROFILE_LINE}/${NEW_PROFILE_LINE}/g" "${PBXPROJECT_PATH}"
}

function UUUpdateProvisioningStyle
{
    if [ $# != 1 ]
    then
        echo "Usage: UUUpdateProvisioningStyle [Xcode project path]"
        exit 1
    fi

    local PROJECT_PATH=$1
    local PBXPROJECT_PATH="${PROJECT_PATH}/project.pbxproj"

	OLD_LINE="ProvisioningStyle = Automatic;"
	NEW_LINE="ProvisioningStyle = Manual;"

	sed -i "" -e "s/${OLD_LINE}/${NEW_LINE}/g" "${PBXPROJECT_PATH}"
}

function UUUpdateProductBundleIdentifier
{
    if [ $# != 2 ]
    then
        echo "Usage: UUUpdateProductBundleIdentifier [Xcode project path] [bundle id]"
        exit 1
    fi

    local PROJECT_PATH=$1
    local BUNDLE_ID=$2
    
	local PBXPROJECT_PATH="${PROJECT_PATH}/project.pbxproj"

	BUNDLE_ID_LINES=`cat "${PBXPROJECT_PATH}" | grep "PRODUCT_BUNDLE_IDENTIFIER"`
	#UUDebugLog "BUNDLE_ID_LINES: ${BUNDLE_ID_LINES}"

	IFS=$'\n' ARR=(${BUNDLE_ID_LINES})
	
	for i in "${ARR[@]}"; do
		#UUDebugLog "LINE: ${i}"
		UUUpdateProductBundleIdentifierLine "${PROJECT_PATH}" "${i}" "${BUNDLE_ID}"
	done

}

function UUUpdateProductBundleIdentifierLine
{
    if [ $# != 3 ]
    then
        echo "Usage: UUUpdateProductBundleIdentifier [Xcode project path] [old bundle id line] [bundle id]"
        exit 1
    fi

    local PROJECT_PATH=$1
    local OLD_BUNDLE_ID_LINE=$2
    local BUNDLE_ID=$3

    local PBXPROJECT_PATH="${PROJECT_PATH}/project.pbxproj"

    ORIGINAL_OLD_BUNDLE_ID_LINE=`UUTrimWhitespace "${OLD_BUNDLE_ID_LINE}"`
    
     declare -a SUFFIX_TO_PRESERVE=("msg" "watchkitapp" "watchkitapp.watchkitextension")
    
    for i in ${SUFFIX_TO_PRESERVE[@]}; 
    do
    	ENDING=".${i};"
    	UUDebugLog "Checking ending ${ENDING}"
    	
		if [[ "${ORIGINAL_OLD_BUNDLE_ID_LINE}" == *${ENDING} ]]
		then
			UUDebugLog "Preserving ${ENDING} bundle suffix"
		
			NEW_BUNDLE_ID_LINE="PRODUCT_BUNDLE_IDENTIFIER = ${BUNDLE_ID}.${i};"
			UUDebugLog "OLD_BUNDLE_ID_LINE: ${OLD_BUNDLE_ID_LINE}"
			UUDebugLog "NEW_BUNDLE_ID_LINE: ${NEW_BUNDLE_ID_LINE}"
			sed -i "" -e "s/${OLD_BUNDLE_ID_LINE}/${NEW_BUNDLE_ID_LINE}/g" "${PBXPROJECT_PATH}"
		fi
	
    done
    
    NEW_BUNDLE_ID_LINE="PRODUCT_BUNDLE_IDENTIFIER = ${BUNDLE_ID};"
	UUDebugLog "OLD_BUNDLE_ID_LINE: ${OLD_BUNDLE_ID_LINE}"
	UUDebugLog "NEW_BUNDLE_ID_LINE: ${NEW_BUNDLE_ID_LINE}"
	sed -i "" -e "s/${OLD_BUNDLE_ID_LINE}/${NEW_BUNDLE_ID_LINE}/g" "${PBXPROJECT_PATH}"
}

function UUStripSimulatorSlices
{
    if [ $# != 1 ]
    then
        echo "Usage: UUStripSimulatorSlices [Binary Path]"
    exit 1
    fi

    local BINARY_PATH=$1

    THIN_PATH="${BINARY_PATH}_thin"

    lipo "${BINARY_PATH}" -remove i386 -remove x86_64 -output "${THIN_PATH}"
	UUCheckReturnCode $? "UUStripSimulatorSlices lipo remove failed"

    rm -rf "${BINARY_PATH}"
	UUCheckReturnCode $? "UUStripSimulatorSlices remove fat binary failed"

    mv "${THIN_PATH}" "${BINARY_PATH}"
	UUCheckReturnCode $? "UUStripSimulatorSlices copy thin binary failed"
}

function UUGetFullPlistPath 
{
	if [ $# -lt 3 ]
	then
		echo "Usage: UUGetFullPlistPath [full path to xcode project] [configuration] [output variable] "
		exit 1
	fi
	
	local PROJECT_PATH=$1
	local CONFIGURATION=$2
	local OUTPUT_VAR=$3
	
	UUExtractXcodeBuildSetting "${PROJECT_PATH}" "${CONFIGURATION}" "PROJECT_DIR" PROJECT_DIR
	UUExtractXcodeBuildSetting "${PROJECT_PATH}" "${CONFIGURATION}" "INFOPLIST_FILE" INFO_PLIST_FILE
	
	UUDebugLog "UUGetFullPlistPath-PROJECT_DIR: ${PROJECT_DIR}"
	UUDebugLog "UUGetFullPlistPath-INFO_PLIST_FILE: ${INFO_PLIST_FILE}"
	
	PLIST_PATH="${PROJECT_DIR}/${INFO_PLIST_FILE}"
	if [[ ${INFO_PLIST_FILE} == /* ]]
	then
		UUDebugLog "UUGetFullPlistPath-INFO_PLIST_FILE is absolute path, don't append project path"
		PLIST_PATH="${INFO_PLIST_FILE}"
	fi
	
	eval $OUTPUT_VAR="'${PLIST_PATH}'"
}

function UUSetBuildNumber
{
	if [ $# -lt 3 ]
	then
		echo "Usage: UUSetBuildNumber [full path to xcode project] [configuration] [fixed version] "
		exit 1
	fi
	
	local PROJECT_PATH=$1
	local CONFIGURATION=$2
	BUILD_VERSION=$3
	
	UUGetFullPlistPath "${PROJECT_PATH}" "${CONFIGURATION}" FULL_PLIST_PATH
	UUDebugLog "UUSetBuildNumber-FULL_PLIST_PATH: ${FULL_PLIST_PATH}"
	
	UURevertGitChanges "${FULL_PLIST_PATH}"
	
	if [ -z ${BUILD_VERSION} ] # Empty version string 
	then
	
		UUReadBundleShortVersionString "${FULL_PLIST_PATH}" CURRENT_VERSION
		
		BUILD_VERSION=${CURRENT_VERSION}
		
		UUIsGitRepo IS_GIT_REPO
		
		if [ ${IS_GIT_REPO} == 1 ]
		then
			UUReadGitRevisionNumber GIT_REV_NUMBER
			BUILD_VERSION="${CURRENT_VERSION}.${GIT_REV_NUMBER}"
		fi
		
		UUDebugLog "UUSetBuildNumber-CURRENT_VERSION=${CURRENT_VERSION}"
		UUDebugLog "UUSetBuildNumber-GIT_REV_NUMBER=${GIT_REV_NUMBER}"
		UUDebugLog "UUSetBuildNumber-BUILD_VERSION=${BUILD_VERSION}"

	else
	
		UUWriteBundleVersionShort "${FULL_PLIST_PATH}" "${BUILD_VERSION}"

	fi
	
	UUWriteBundleVersion "${PLIST_PATH}" "${BUILD_VERSION}" 
}

function UUMakeArchive
{
	if [ $# -lt 6 ]
	then
		echo "Usage: UUMakeArchive [full path to xcode project] [scheme name] [full path to export plist] [crashlytics group alias] [crashlytics notes path] [output dir]  "
		exit 1
	fi

	CWD=$(pwd)

	local PROJECT_PATH=$1
	local SCHEME_NAME=$2
	local EXPORT_PLIST_FILE=$3
	local CRASHLYTICS_GROUP_ALIAS=$4
	local CRASHLYTICS_NOTES_PATH=$5
	local OUTPUT_DIR=$6

	CONFIG_NAME="${SCHEME_NAME}"
	
    UUExtractXcodeBuildSetting "${PROJECT_PATH}" "${CONFIG_NAME}" "FULL_PRODUCT_NAME" FULL_PRODUCT_NAME
    UUDebugLog "FULL_PRODUCT_NAME: ${FULL_PRODUCT_NAME}"

    FULL_PRODUCT_NAME_NO_SPACES=`UURemoveSpaces "${FULL_PRODUCT_NAME}"`
	
	PROJECT_NAME_NO_SPACES="${FULL_PRODUCT_NAME_NO_SPACES/.app/}"
	
	OUTPUT_FILE_NAME="${PROJECT_NAME_NO_SPACES}.xcarchive"
	OUTPUT_ARCHIVE_PATH="${OUTPUT_DIR}/${OUTPUT_FILE_NAME}"
	OUTPUT_ARCHIVE_ZIP_PATH="${PROJECT_NAME_NO_SPACES}_${BUILD_VERSION}_${CONFIG_NAME}.xcarchive.zip"

	UUGenerateArchive "${PROJECT_PATH}" "${SCHEME_NAME}" "${CONFIG_NAME}" "${OUTPUT_ARCHIVE_PATH}"
	UUMakeIpa "${OUTPUT_ARCHIVE_PATH}" "${EXPORT_PLIST_FILE}" "${OUTPUT_DIR}"
	
	UUZipFolder "${OUTPUT_ARCHIVE_PATH}" "${OUTPUT_ARCHIVE_ZIP_PATH}"
	UUDeleteFile "${OUTPUT_ARCHIVE_PATH}"
	
	OUTPUT_IPA_NAME="${PROJECT_NAME_NO_SPACES}_${BUILD_VERSION}_${CONFIG_NAME}.ipa"
	
	mv "${OUTPUT_DIR}/${SCHEME_NAME}.ipa" "${OUTPUT_DIR}/${OUTPUT_IPA_NAME}"
	
	FULL_OUTPUT_IPA_NAME="${OUTPUT_DIR}/${OUTPUT_IPA_NAME}"
	
	UUUploadIpaToCrashlytics "${PROJECT_PATH}" "${FULL_OUTPUT_IPA_NAME}" "${CRASHLYTICS_GROUP_ALIAS}" "${CRASHLYTICS_NOTES_PATH}"
}

function UUMakeIpa
{
	if [ $# -lt 3 ]
	then
		echo "Usage: UUMakeIpa [full path to archive] [full path to export plist] [output dir] "
		exit 1
	fi
	
	local ARCHIVE_PATH=$1
	local EXPORT_PLIST_FILE=$2
	local OUTPUT_DIR=$3

	xcodebuild \
		-exportArchive \
		-archivePath "${ARCHIVE_PATH}" \
		-exportOptionsPlist "${EXPORT_PLIST_FILE}" \
		-exportPath "${OUTPUT_DIR}"

	UUCheckReturnCode $? "Make IPA"
}

function UUUploadIpaToCrashlytics
{
	if [ $# -lt 4 ]
	then
		echo "Usage: UUUploadProjectToCrashlytics [full path to xcode project] [full ipa path] [crashlytics group alias] [crashlytics notes path] "
		exit 1
	fi
	
	local PROJECT_PATH=$1
	local IPA_FULL_PATH=$2
	local CRASHLYTICS_GROUP_ALIAS=$3
	local CRASHLYTICS_NOTES_PATH=$4
	
	if [ ! -z ${CRASHLYTICS_GROUP_ALIAS} ]
	then
	
		UUExtractCrashlyticsKeyAndSecret "${PROJECT_PATH}" CRASHLYTICS_API_KEY CRASHLYTICS_BUILD_SECRET
		
		UUFindFolder "../" "Crashlytics.framework" CRASHLYTICS_PATH
		
		COCOAPODS_CRASHLYTICS_PATH=../Pods/Crashlytics
		UUDebugLog "COCOAPODS_CRASHLYTICS_PATH=${COCOAPODS_CRASHLYTICS_PATH}"
		
		if [ -d "${COCOAPODS_CRASHLYTICS_PATH}" ]
		then
			UUDebugLog "Crashlytics folder found under cocoa pods, using fixed path for submit file"
			CRASHLYTICS_PATH=${COCOAPODS_CRASHLYTICS_PATH}
		fi

		UUDebugLog "CRASHLYTICS_PATH: ${CRASHLYTICS_PATH}"
		UUDebugLog "CRASHLYTICS_API_KEY: ${CRASHLYTICS_API_KEY}"
		UUDebugLog "CRASHLYTICS_BUILD_SECRET: ${CRASHLYTICS_BUILD_SECRET}"
		UUDebugLog "CRASHLYTICS_GROUP_ALIAS: ${CRASHLYTICS_GROUP_ALIAS}"
		UUDebugLog "CRASHLYTICS_NOTES_PATH: ${CRASHLYTICS_NOTES_PATH}"
		UUDebugLog "IPA_FULL_PATH: ${IPA_FULL_PATH}"

		if [ ! -z ${CRASHLYTICS_GROUP_ALIAS} ]
		then
			UUUploadToCrashlytics "${CRASHLYTICS_API_KEY}" "${CRASHLYTICS_BUILD_SECRET}" "${CRASHLYTICS_PATH}" "${CRASHLYTICS_GROUP_ALIAS}" "${CRASHLYTICS_NOTES_PATH}" "${IPA_FULL_PATH}"
		fi
	fi
}

function UUMakeBuildFixed
{
	if [ $# -lt 9 ]
	then
		echo "Usage: UUMakeBuildFixed [full path to xcode project] [scheme name] [full path to export plist] [cert path] [cert password] [crashlytics group alias] [crashlytics notes path] [fixed version] [output dir]  "
		exit 1
	fi
	
	local PROJECT_PATH=$1
	local SCHEME_NAME=$2
	local EXPORT_PLIST_FILE=$3
	local CERT_PATH=$4
	local CERT_PASSWORD=$5
	local CRASHLYTICS_GROUP_ALIAS=$6
	local CRASHLYTICS_NOTES_PATH=$7
	local FIXED_VERSION=$8
	local OUTPUT_DIR=$9
	
	TMP_KEYCHAIN_PATH=""

	UUExtractFolder "${PROJECT_PATH}" REVERT_FOLDER
	UUDebugLog "Revert Folder: ${REVERT_FOLDER}"

	trap 'UUCleanupAfterBuild ${TMP_KEYCHAIN_PATH} ${REVERT_FOLDER}' EXIT

	UUSetBuildNumber "${PROJECT_PATH}" "${SCHEME_NAME}" "${FIXED_VERSION}"
	UUMakeTempKeychain "${CERT_PATH}" "${CERT_PASSWORD}" TMP_KEYCHAIN_PATH
	UUMakeArchive "${PROJECT_PATH}" "${SCHEME_NAME}" "${EXPORT_PLIST_FILE}" "${CRASHLYTICS_GROUP_ALIAS}" "${CRASHLYTICS_NOTES_PATH}" "${OUTPUT_DIR}"
}

function UUCleanupAfterBuild
{
	UUDebugLog "Doing final cleanup"
	
	KEYCHAIN_PATH=$1
	REVERT_PATH=$2
	
	if [ ! -z "${KEYCHAIN_PATH}" ]
	then
		UUDebugLog "Cleaning up temporary keychain"
		UUCleanupBuildKeychain "${KEYCHAIN_PATH}" -v 
	else
		UUDebugLog "No keychain to cleanup"
	fi
	
	UUDebugLog "Reverting code, REVERT_PATH: ${REVERT_PATH}"
	UURevertChanges "${REVERT_PATH}"
}

function UUMakeBuild
{
	if [ $# -lt 7 ]
	then
		echo "Usage: UUMakeBuild [full path to xcode project] [scheme name] [path to provisioning profile] [path to certificate] [certificate password] [output dir] [ipa export type] [fixed version] [crashlytics group alias] [crashlytics notes path] "
		exit 1
	fi

	CWD=$(pwd)

	local PROJECT_PATH=$1
	local SCHEME_NAME=$2
	local PROVISION_PROFILE_PATH=$3
	local CERT_PATH=$4
	local CERT_PASSWORD=$5
	local OUTPUT_DIR=$6
	local IPA_EXPORT_TYPE=$7
	local BUILD_VERSION=$8
	local CRASHLYTICS_GROUP_ALIAS=$9
	local CRASHLYTICS_NOTES_PATH=${10}
	
	local UPLOAD_TO_CRASHLYTICS=0
	
	if [ ! -z ${CRASHLYTICS_GROUP_ALIAS} ]
	then
		UPLOAD_TO_CRASHLYTICS=1
	fi
	
	local HAS_FIXED_VERSION=0
	
	if [ ! -z ${BUILD_VERSION} ]
	then
		HAS_FIXED_VERSION=1
	fi
	
	UUDebugLog "CRASHLYTICS_GROUP_ALIAS: ${CRASHLYTICS_GROUP_ALIAS}"
	UUDebugLog "UPLOAD_TO_CRASHLYTICS: ${UPLOAD_TO_CRASHLYTICS}"
	
	UUDebugLog "BUILD_VERSION: ${BUILD_VERSION}"
	UUDebugLog "HAS_FIXED_VERSION: ${HAS_FIXED_VERSION}"

	PROVISION_PROFILE_LOC=~/Library/MobileDevice/Provisioning\ Profiles/
	ARCHIVE_CONFIGURATION=Release
	
	UUExtractFileName "${PROJECT_PATH}" PROJECT_NAME_WITH_EXTENSION
	UUExtractFileNameNoExtension "${PROJECT_PATH}" PROJECT_NAME
	UUExtractFileExtension "${PROJECT_PATH}" PROJECT_EXTENSION
	UUExtractFolder "${PROJECT_PATH}" PROJECT_FOLDER
	UUExtractXcodeBuildSetting "${PROJECT_PATH}" "${SCHEME_NAME}" "PROJECT_DIR" PROJECT_DIR
	UUExtractXcodeBuildSetting "${PROJECT_PATH}" "${SCHEME_NAME}" "PROJECT_FILE_PATH" PROJECT_FILE_PATH
	UUExtractXcodeBuildSetting "${PROJECT_PATH}" "${SCHEME_NAME}" "INFOPLIST_FILE" INFO_PLIST_FILE
    UUExtractXcodeBuildSetting "${PROJECT_PATH}" "${SCHEME_NAME}" "FULL_PRODUCT_NAME" FULL_PRODUCT_NAME
    UUDebugLog "PROJECT_DIR: ${PROJECT_DIR}"
    UUDebugLog "PROJECT_FILE_PATH: ${PROJECT_FILE_PATH}"
    UUDebugLog "INFO_PLIST_FILE: ${INFO_PLIST_FILE}"
    UUDebugLog "FULL_PRODUCT_NAME: ${FULL_PRODUCT_NAME}"
    
	local PBXPROJECT_PATH="${PROJECT_FILE_PATH}/project.pbxproj"
	
	UUIsGitRepo IS_GIT_REPO
	UUIsSvnRepo IS_SVN_REPO
	
	UUDebugLog "IS_GIT_REPO: ${IS_GIT_REPO}"
	UUDebugLog "IS_SVN_REPO: ${IS_SVN_REPO}"
	
    FULL_PRODUCT_NAME_NO_SPACES=`UURemoveSpaces "${FULL_PRODUCT_NAME}"`
	
	PLIST_PATH="${PROJECT_DIR}/${INFO_PLIST_FILE}"
	if [[ ${INFO_PLIST_FILE} == /* ]]
	then
		UUDebugLog "INFO_PLIST_FILE is absolute path, don't append project path"
		PLIST_PATH="${INFO_PLIST_FILE}"
	fi
	
	PROJECT_NAME_NO_SPACES="${FULL_PRODUCT_NAME_NO_SPACES/.app/}"
	PROJECT_NAME_NO_SPACES_NO_UNDERSCORES="${PROJECT_NAME_NO_SPACES//_/}"
	
	UUExtractFileName "${PROVISION_PROFILE_PATH}" PROFILE_NAME_WITH_EXTENSION
	PROVISION_PROFILE_DEST_PATH="${PROVISION_PROFILE_LOC}${PROFILE_NAME_WITH_EXTENSION}"
	
	UUDebugLog "PROJECT_PATH: ${PROJECT_PATH}"
	UUDebugLog "PROJECT_NAME_WITH_EXTENSION: ${PROJECT_NAME_WITH_EXTENSION}"
	UUDebugLog "PROJECT_NAME: ${PROJECT_NAME}"
	UUDebugLog "PROJECT_EXTENSION: ${PROJECT_EXTENSION}"
	UUDebugLog "PROJECT_FOLDER: ${PROJECT_FOLDER}"
	UUDebugLog "INFO_PLIST_FILE: ${INFO_PLIST_FILE}"
	UUDebugLog "PLIST_PATH: ${PLIST_PATH}"
	UUDebugLog "PROJECT_NAME_NO_SPACES: ${PROJECT_NAME_NO_SPACES}"
	UUDebugLog "PROJECT_NAME_NO_SPACES_NO_UNDERSCORES: ${PROJECT_NAME_NO_SPACES_NO_UNDERSCORES}"
	UUDebugLog "PROFILE_NAME_WITH_EXTENSION: ${PROFILE_NAME_WITH_EXTENSION}"
	UUDebugLog "ARCHIVE_CONFIGURATION: ${ARCHIVE_CONFIGURATION}"
	
	if [ ${IS_GIT_REPO} == 1 ]
	then
		UURevertGitChanges "${PLIST_PATH}"
		UURevertGitChanges "${PROJECT_FILE_PATH}"
	fi
	
	if [ ${IS_SVN_REPO} == 1 ]
	then
		UURevertSvnChanges "${PLIST_PATH}"
		UURevertSvnChanges "${PBXPROJECT_PATH}"
	fi
	
	UUExtractMobileProvisionValue "${PROVISION_PROFILE_PATH}" "TeamName" TEAM_NAME
	UUExtractMobileProvisionValue "${PROVISION_PROFILE_PATH}" "ApplicationIdentifierPrefix:0" APP_IDENTIFIER_PREFIX
	UUExtractMobileProvisionValue "${PROVISION_PROFILE_PATH}" "TeamIdentifier:0" TEAM_IDENTIFIER
	UUExtractMobileProvisionValue "${PROVISION_PROFILE_PATH}" ":Entitlements:application-identifier" APP_IDENTIFIER
	UUExtractMobileProvisionValue "${PROVISION_PROFILE_PATH}" "Name" PROVISION_PROFILE_NAME
	UUExtractMobileProvisionValue "${PROVISION_PROFILE_PATH}" "UUID" PROVISION_PROFILE_GUID
	
	CODE_SIGNING_IDENTITY="iPhone Distribution: ${TEAM_NAME}"
	
	UUDebugLog "TEAM_NAME: ${TEAM_NAME}"
	UUDebugLog "TEAM_IDENTIFIER: ${TEAM_IDENTIFIER}"
	UUDebugLog "APP_IDENTIFIER_PREFIX: ${APP_IDENTIFIER_PREFIX}"
	UUDebugLog "APP_IDENTIFIER: ${APP_IDENTIFIER}"
	UUDebugLog "PROVISION_PROFILE_NAME: ${PROVISION_PROFILE_NAME}"
	UUDebugLog "PROVISION_PROFILE_GUID: ${PROVISION_PROFILE_GUID}"
	
	APP_IDENTIFIER_PREFIX_WITH_DOT="${APP_IDENTIFIER_PREFIX}."
	BUNDLE_ID="${APP_IDENTIFIER/${APP_IDENTIFIER_PREFIX_WITH_DOT}/}"
	UUDebugLog "BUNDLE_ID: ${BUNDLE_ID}"
	
	if [[ ${BUNDLE_ID} == *"*"* ]]
	then
		BUNDLE_ID="${BUNDLE_ID/'*'/${PROJECT_NAME_NO_SPACES_NO_UNDERSCORES}}"
	fi
	
	UUDebugLog "BUNDLE_ID: ${BUNDLE_ID}"
	
	if [ ${HAS_FIXED_VERSION} == 0 ]
	then
		UUReadBundleShortVersionString "${PLIST_PATH}" CURRENT_VERSION
		
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
	
	if [ ${HAS_FIXED_VERSION} == 1 ]
	then
		UUWriteBundleVersionShort "${PLIST_PATH}" "${BUILD_VERSION}"
	fi
	
	# do this before mucking with bundle id changes
	UUExtractCrashlyticsKeyAndSecret "${PROJECT_FILE_PATH}" CRASHLYTICS_API_KEY CRASHLYTICS_BUILD_SECRET
	
	UUWriteBundleVersion "${PLIST_PATH}" "${BUILD_VERSION}" 
    #UUWriteBundleIdentifier "${PLIST_PATH}" "${BUNDLE_ID}"
    UUUpdateProductBundleIdentifier "${PROJECT_FILE_PATH}" "${BUNDLE_ID}"
    UUUpdateProvisioningProfile "${PROJECT_FILE_PATH}" "${CODE_SIGNING_IDENTITY}" "${PROVISION_PROFILE_GUID}" "${PROVISION_PROFILE_NAME}"
    UUUpdateTeamIdentifier "${PROJECT_FILE_PATH}" "${TEAM_IDENTIFIER}"
    UUUpdateProvisioningStyle "${PROJECT_FILE_PATH}"
    
    if [ ! -z "${UU_WATCH_EXT_PLIST_PATH}" ]
    then
		UUDebugLog "Updating watch app extension plist"
		WATCH_BUNDLE_ID="${BUNDLE_ID}.watchkitapp"
		UUDebugLog "UU_WATCH_EXT_PLIST_PATH: ${UU_WATCH_EXT_PLIST_PATH}"
		UUDebugLog "WATCH_BUNDLE_ID: ${WATCH_BUNDLE_ID}"
		UUUpdateWatchAppBundleIdentifier "${UU_WATCH_EXT_PLIST_PATH}" "${WATCH_BUNDLE_ID}"
	fi
	
	if [ ! -z "${UU_WATCH_PLIST}" ]
    then
		UUDebugLog "Updating watch app plist"
		UUWritePlistString "${UU_WATCH_PLIST}" "WKCompanionAppBundleIdentifier" "${BUNDLE_ID}"
	fi
	
	KEYCHAIN_NAME=${PROJECT_NAME_NO_SPACES}-${BUILD_VERSION}
	KEYCHAIN_PATH=~/Library/Keychains/${KEYCHAIN_NAME}.keychain
	KEYCHAIN_PASS=masterpassword
	UUDebugLog "KEYCHAIN_NAME: ${KEYCHAIN_NAME}"
	
	UUSetupBuildKeychain ${KEYCHAIN_PATH} ${KEYCHAIN_PASS} ${CERT_PATH} ${CERT_PASSWORD} -v

	UUCopyFile "${PROVISION_PROFILE_PATH}" "${PROVISION_PROFILE_DEST_PATH}"

	OUTPUT_FILE_NAME="${PROJECT_NAME_NO_SPACES}_${BUILD_VERSION}_${IPA_EXPORT_TYPE}"
	OUTPUT_ARCHIVE_PATH="${OUTPUT_DIR}/${OUTPUT_FILE_NAME}.xcarchive.zip"
	OUTPUT_ARCHIVE_TEMP_PATH="/tmp/${OUTPUT_FILE_NAME}.xcarchive"
	OUTPUT_IPA_PATH="${OUTPUT_DIR}"
	OUTPUT_IPA_FULL_PATH="${OUTPUT_DIR}/${OUTPUT_FILE_NAME}.ipa"

	UUGenerateArchive "${PROJECT_PATH}" "${SCHEME_NAME}" "${ARCHIVE_CONFIGURATION}" "${OUTPUT_ARCHIVE_TEMP_PATH}"
	
	UUGenerateIpa "${OUTPUT_ARCHIVE_TEMP_PATH}" "${OUTPUT_IPA_PATH}" "${TEAM_IDENTIFIER}" "${IPA_EXPORT_TYPE}"
	
	UUDeleteFile "${OUTPUT_ARCHIVE_PATH}"
	UUZipFolder "${OUTPUT_ARCHIVE_TEMP_PATH}" "${OUTPUT_ARCHIVE_PATH}"

	UU_IPA_OUTPUT_PATH="${OUTPUT_DIR}/${OUTPUT_FILE_NAME}.ipa"
	UU_IPA_OUTPUT_PATH_NO_VERSION="${UU_IPA_OUTPUT_PATH/${BUILD_VERSION}_/}"
	
	mv "${OUTPUT_DIR}/${SCHEME_NAME}.ipa" "${UU_IPA_OUTPUT_PATH}"
	
	if [ ${UPLOAD_TO_CRASHLYTICS} == 1 ]
	then
		
		UUFindFolder "../" "Crashlytics.framework" CRASHLYTICS_PATH
		
		COCOAPODS_CRASHLYTICS_PATH=../Pods/Crashlytics
		UUDebugLog "COCOAPODS_CRASHLYTICS_PATH=${COCOAPODS_CRASHLYTICS_PATH}"
		
		if [ -d "${COCOAPODS_CRASHLYTICS_PATH}" ]
		then
			UUDebugLog "Crashlytics folder found under cocoa pods, using fixed path for submit file"
			CRASHLYTICS_PATH=${COCOAPODS_CRASHLYTICS_PATH}
		fi

		UUDebugLog "CRASHLYTICS_PATH: ${CRASHLYTICS_PATH}"
		UUDebugLog "CRASHLYTICS_API_KEY: ${CRASHLYTICS_API_KEY}"
		UUDebugLog "CRASHLYTICS_BUILD_SECRET: ${CRASHLYTICS_BUILD_SECRET}"
		UUDebugLog "CRASHLYTICS_GROUP_ALIAS: ${CRASHLYTICS_GROUP_ALIAS}"
		UUDebugLog "CRASHLYTICS_NOTES_PATH: ${CRASHLYTICS_NOTES_PATH}"
		UUDebugLog "OUTPUT_IPA_FULL_PATH: ${OUTPUT_IPA_FULL_PATH}"

		UUUploadToCrashlytics "${CRASHLYTICS_API_KEY}" "${CRASHLYTICS_BUILD_SECRET}" "${CRASHLYTICS_PATH}" "${CRASHLYTICS_GROUP_ALIAS}" "${CRASHLYTICS_NOTES_PATH}" "${OUTPUT_IPA_FULL_PATH}"
	fi

	UUCleanupBuildKeychain "${KEYCHAIN_PATH}" -v
	
	if [ ${IS_GIT_REPO} == 1 ]
	then
		UURevertGitChanges "${PLIST_PATH}"
		UURevertGitChanges "${PROJECT_FILE_PATH}"
	fi
	
	if [ ${IS_SVN_REPO} == 1 ]
	then
		UURevertSvnChanges "${PLIST_PATH}"
		UURevertSvnChanges "${PBXPROJECT_PATH}"
	fi
	
	UUDeleteFile "${PROVISION_PROFILE_DEST_PATH}"
	UUDeleteFile "${OUTPUT_ARCHIVE_TEMP_PATH}"
}

function UUMakeFrameworkBuild
{
	if [ $# -lt 7 ]
	then
		echo "Usage: UUMakeFrameworkBuild [full path to xcode project] [scheme name] [path to provisioning profile] [path to certificate] [certificate password] [output dir] [fixed version] "
		exit 1
	fi

	CWD=$(pwd)

	local PROJECT_PATH=$1
	local SCHEME_NAME=$2
	local PROVISION_PROFILE_PATH=$3
	local CERT_PATH=$4
	local CERT_PASSWORD=$5
	local OUTPUT_DIR=$6
	local BUILD_VERSION=$7
	
	local HAS_FIXED_VERSION=0
	
	if [ ! -z ${BUILD_VERSION} ]
	then
		HAS_FIXED_VERSION=1
	fi
	
	UUDebugLog "BUILD_VERSION: ${BUILD_VERSION}"
	UUDebugLog "HAS_FIXED_VERSION: ${HAS_FIXED_VERSION}"

	PROVISION_PROFILE_LOC=~/Library/MobileDevice/Provisioning\ Profiles/
	ARCHIVE_CONFIGURATION=Release
	
	UUExtractFileName "${PROJECT_PATH}" PROJECT_NAME_WITH_EXTENSION
	UUExtractFileNameNoExtension "${PROJECT_PATH}" PROJECT_NAME
	UUExtractFileExtension "${PROJECT_PATH}" PROJECT_EXTENSION
	UUExtractFolder "${PROJECT_PATH}" PROJECT_FOLDER
	UUExtractXcodeBuildSetting "${PROJECT_PATH}" "${ARCHIVE_CONFIGURATION}" "PROJECT_DIR" PROJECT_DIR
	UUExtractXcodeBuildSetting "${PROJECT_PATH}" "${ARCHIVE_CONFIGURATION}" "PROJECT_FILE_PATH" PROJECT_FILE_PATH
	UUExtractXcodeBuildSetting "${PROJECT_PATH}" "${ARCHIVE_CONFIGURATION}" "INFOPLIST_FILE" INFO_PLIST_FILE
    UUExtractXcodeBuildSetting "${PROJECT_PATH}" "${ARCHIVE_CONFIGURATION}" "FULL_PRODUCT_NAME" FULL_PRODUCT_NAME

	local PBXPROJECT_PATH="${PROJECT_FILE_PATH}/project.pbxproj"
	
	UUIsGitRepo IS_GIT_REPO
	UUIsSvnRepo IS_SVN_REPO
	
	UUDebugLog "IS_GIT_REPO: ${IS_GIT_REPO}"
	UUDebugLog "IS_SVN_REPO: ${IS_SVN_REPO}"
	
	
    FULL_PRODUCT_NAME_NO_SPACES=`UURemoveSpaces "${FULL_PRODUCT_NAME}"`
	PLIST_PATH="${PROJECT_DIR}/${INFO_PLIST_FILE}"
	PROJECT_NAME_NO_SPACES="${FULL_PRODUCT_NAME_NO_SPACES/.app/}"
	PROJECT_NAME_NO_SPACES_NO_UNDERSCORES="${PROJECT_NAME_NO_SPACES//_/}"
	UUExtractFileName "${PROVISION_PROFILE_PATH}" PROFILE_NAME_WITH_EXTENSION
	PROVISION_PROFILE_DEST_PATH="${PROVISION_PROFILE_LOC}${PROFILE_NAME_WITH_EXTENSION}"
	
	UUDebugLog "PROJECT_PATH: ${PROJECT_PATH}"
	UUDebugLog "PROJECT_NAME_WITH_EXTENSION: ${PROJECT_NAME_WITH_EXTENSION}"
	UUDebugLog "PROJECT_NAME: ${PROJECT_NAME}"
	UUDebugLog "PROJECT_EXTENSION: ${PROJECT_EXTENSION}"
	UUDebugLog "PROJECT_FOLDER: ${PROJECT_FOLDER}"
	UUDebugLog "INFO_PLIST_FILE: ${INFO_PLIST_FILE}"
	UUDebugLog "PLIST_PATH: ${PLIST_PATH}"
	UUDebugLog "PROJECT_NAME_NO_SPACES: ${PROJECT_NAME_NO_SPACES}"
	UUDebugLog "PROJECT_NAME_NO_SPACES_NO_UNDERSCORES: ${PROJECT_NAME_NO_SPACES_NO_UNDERSCORES}"
	UUDebugLog "PROFILE_NAME_WITH_EXTENSION: ${PROFILE_NAME_WITH_EXTENSION}"
	UUDebugLog "ARCHIVE_CONFIGURATION: ${ARCHIVE_CONFIGURATION}"
	
	if [ ${IS_GIT_REPO} == 1 ]
	then
		UURevertGitChanges "${PLIST_PATH}"
		UURevertGitChanges "${PROJECT_FILE_PATH}"
	fi
	
	if [ ${IS_SVN_REPO} == 1 ]
	then
		UURevertSvnChanges "${PLIST_PATH}"
		UURevertSvnChanges "${PBXPROJECT_PATH}"
	fi
	
	UUExtractMobileProvisionValue "${PROVISION_PROFILE_PATH}" "TeamName" TEAM_NAME
	UUExtractMobileProvisionValue "${PROVISION_PROFILE_PATH}" "ApplicationIdentifierPrefix:0" APP_IDENTIFIER_PREFIX
	UUExtractMobileProvisionValue "${PROVISION_PROFILE_PATH}" "TeamIdentifier:0" TEAM_IDENTIFIER
	UUExtractMobileProvisionValue "${PROVISION_PROFILE_PATH}" ":Entitlements:application-identifier" APP_IDENTIFIER
	UUExtractMobileProvisionValue "${PROVISION_PROFILE_PATH}" "Name" PROVISION_PROFILE_NAME
	UUExtractMobileProvisionValue "${PROVISION_PROFILE_PATH}" "UUID" PROVISION_PROFILE_GUID
	
	CODE_SIGNING_IDENTITY="iPhone Distribution: ${TEAM_NAME}"
	
	UUDebugLog "TEAM_NAME: ${TEAM_NAME}"
	UUDebugLog "TEAM_IDENTIFIER: ${TEAM_IDENTIFIER}"
	UUDebugLog "APP_IDENTIFIER_PREFIX: ${APP_IDENTIFIER_PREFIX}"
	UUDebugLog "APP_IDENTIFIER: ${APP_IDENTIFIER}"
	UUDebugLog "PROVISION_PROFILE_NAME: ${PROVISION_PROFILE_NAME}"
	UUDebugLog "PROVISION_PROFILE_GUID: ${PROVISION_PROFILE_GUID}"

	APP_IDENTIFIER_PREFIX_WITH_DOT="${APP_IDENTIFIER_PREFIX}."
	BUNDLE_ID="${APP_IDENTIFIER/${APP_IDENTIFIER_PREFIX_WITH_DOT}/}"
	UUDebugLog "BUNDLE_ID: ${BUNDLE_ID}"
	
	if [[ ${BUNDLE_ID} == *"*"* ]]
	then
		BUNDLE_ID="${BUNDLE_ID/'*'/${PROJECT_NAME_NO_SPACES_NO_UNDERSCORES}}"
	fi
	
	UUDebugLog "BUNDLE_ID: ${BUNDLE_ID}"
	
	if [ ${HAS_FIXED_VERSION} == 0 ]
	then
		UUReadBundleShortVersionString "${PLIST_PATH}" CURRENT_VERSION
		
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
	
	if [ ${HAS_FIXED_VERSION} == 1 ]
	then
		UUWriteBundleVersionShort "${PLIST_PATH}" "${BUILD_VERSION}"
	fi
	
	UUWriteBundleVersion "${PLIST_PATH}" "${BUILD_VERSION}"
    UUWriteBundleIdentifier "${PLIST_PATH}" "${BUNDLE_ID}"
    UUUpdateProductBundleIdentifier "${PROJECT_FILE_PATH}" "${BUNDLE_ID}"
    UUUpdateProvisioningProfile "${PROJECT_FILE_PATH}" "${CODE_SIGNING_IDENTITY}" "${PROVISION_PROFILE_GUID}" "${PROVISION_PROFILE_NAME}"

	KEYCHAIN_NAME=${PROJECT_NAME_NO_SPACES}-${BUILD_VERSION}
	KEYCHAIN_PATH=~/Library/Keychains/${KEYCHAIN_NAME}.keychain
	KEYCHAIN_PASS=masterpassword
	UUDebugLog "KEYCHAIN_NAME: ${KEYCHAIN_NAME}"
	
	UUSetupBuildKeychain ${KEYCHAIN_PATH} ${KEYCHAIN_PASS} ${CERT_PATH} ${CERT_PASSWORD} -v

	UUCopyFile "${PROVISION_PROFILE_PATH}" "${PROVISION_PROFILE_DEST_PATH}"


    TMP_OUTPUT_PATH="/tmp/${SCHEME_NAME}-slices"
    rm -rf "${TMP_OUTPUT_PATH}"
    mkdir "${TMP_OUTPUT_PATH}"

    SDK=iphoneos
    SLICE_OUTPUT="${TMP_OUTPUT_PATH}/${SDK}"
    mkdir "${SLICE_OUTPUT}"
    INPUT_A="${SLICE_OUTPUT}/${FULL_PRODUCT_NAME}/${PROJECT_NAME}"
    LIB_OUTPUT="${SLICE_OUTPUT}/${FULL_PRODUCT_NAME}"
    UUGenerateBuild "${PROJECT_PATH}" "${SCHEME_NAME}" "${SDK}" "${ARCHIVE_CONFIGURATION}" "${SLICE_OUTPUT}"

    SDK=iphonesimulator
    SLICE_OUTPUT="${TMP_OUTPUT_PATH}/${SDK}"
    mkdir "${SLICE_OUTPUT}"
    INPUT_B="${SLICE_OUTPUT}/${FULL_PRODUCT_NAME}/${PROJECT_NAME}"
    UUGenerateBuild "${PROJECT_PATH}" "${SCHEME_NAME}" "${SDK}" "${ARCHIVE_CONFIGURATION}" "${SLICE_OUTPUT}"

    FAT_OUTPUT_FILE="${TMP_OUTPUT_PATH}/${PROJECT_NAME}"
    lipo "${INPUT_A}" "${INPUT_B}" -create -output "${FAT_OUTPUT_FILE}"

    rm -rf "${INPUT_A}"
    cp "${FAT_OUTPUT_FILE}" "${INPUT_A}"

    cp -av "${LIB_OUTPUT}" "${OUTPUT_DIR}"

    rm -rf "${TMP_OUTPUT_PATH}"

	UUCleanupBuildKeychain "${KEYCHAIN_PATH}" -v
	
	if [ ${IS_GIT_REPO} == 1 ]
	then
		UURevertGitChanges "${PLIST_PATH}"
		UURevertGitChanges "${PROJECT_FILE_PATH}"
	fi
	
	if [ ${IS_SVN_REPO} == 1 ]
	then
		UURevertSvnChanges "${PLIST_PATH}"
		UURevertSvnChanges "${PBXPROJECT_PATH}"
	fi
	
	UUDeleteFile "${PROVISION_PROFILE_DEST_PATH}"
}
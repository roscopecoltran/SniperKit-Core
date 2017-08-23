#!/bin/sh

function UUDebugLog
{
	LINE=$1
	
	# Uncomment this line to print debug info
	echo "${LINE}"
}

function UUCheckReturnCode 
{
	if [ $1 != 0 ]
	then
		echo "Return Code check failed for action $2, exiting with error code $1"
		exit $1
	fi
}

function UURemoveSpaces
{
	local INPUT=$1
	echo ${INPUT//[[:blank:]]/}
}

function UUTrimWhitespace
{
    local INPUT=$1
    NO_SPACE="$(echo "${INPUT}" | sed -e 's/^[[:space:]]*//' -e 's/[[:space:]]*$//')"
    echo ${NO_SPACE}
}

function UUIsGitRepo
{
	if [ $# != 1 ]
	then
		echo "Usage: UUIsGitRepo [Output Variable]"
		exit 1
	fi
	
	local OUTPUT_VAR=$1
	
	eval $OUTPUT_VAR=1
	
	git rev-parse --is-inside-work-tree
	
	if [ $? != 0 ]
	then
		eval $OUTPUT_VAR=0
	fi
}

function UUIsSvnRepo
{
	if [ $# != 1 ]
	then
		echo "Usage: UUIsSvnRepo [Output Variable]"
		exit 1
	fi
	
	local OUTPUT_VAR=$1
	
	eval $OUTPUT_VAR=1
	
	svn info
	
	if [ $? != 0 ]
	then
		eval $OUTPUT_VAR=0
	fi
}

# This method will replace the last digit in a dotted version string with a fixed number.
# The intention is that the project plist will have either a three or four digit build number
function UUGenerateBuildNumber
{
	if [ $# != 3 ]
	then
		echo "Usage: UUGenerateBuildNumber [Current Version String] [Build Number] [Output Variable]"
		exit 1
	fi
	
	local CURRENT_VERSION=$1
	local BUILD_NUMBER=$2
	local OUTPUT_VAR=$3
	
	PARTS=()

	IFS='.' read -ra ARR <<< "${CURRENT_VERSION}"
	PART_COUNT=0
	for i in "${ARR[@]}"; do
		COMPONENT=${i}
		PARTS+=(${COMPONENT})
		let PART_COUNT=PART_COUNT+1
	done

	let PART_COUNT=PART_COUNT-1
	
	if (( ${PART_COUNT} <= 0 ))
	then
		echo "UUGenerateBuildNumber expects current version to have at least 1 components"
		exit 1
	fi

	NEW_VERSION=''
	for i in `seq 1 ${PART_COUNT}`;
	do
		let INDEX=i-1
		PART=${PARTS[${INDEX}]}
		NEW_VERSION+="${PART}."
	done   

	NEW_VERSION+="${BUILD_NUMBER}"

	eval $OUTPUT_VAR="'${NEW_VERSION}'"
}

# Slices off the fourth part of a build number if needed
function UUGetShortBuildNumber
{
	if [ $# != 2 ]
	then
		echo "Usage: UUGetShortBuildNumber [Version String] [Output Variable]"
		exit 1
	fi
	
	local CURRENT_VERSION=$1
	local OUTPUT_VAR=$2
	
	PARTS=()

	IFS='.' read -ra ARR <<< "${CURRENT_VERSION}"
	PART_COUNT=0
	for i in "${ARR[@]}"; do
		COMPONENT=${i}
		PARTS+=(${COMPONENT})
		let PART_COUNT=PART_COUNT+1
	done

	let PART_COUNT=PART_COUNT-1
	
	if (( ${PART_COUNT} <= 3 ))
	then
		eval $OUTPUT_VAR="'${CURRENT_VERSION}'"
	fi

	let PERIOD_STOP=PART_COUNT-1
	NEW_VERSION=''
	for i in `seq 1 ${PART_COUNT}`;
	do
		let INDEX=i-1
		PART=${PARTS[${INDEX}]}
		NEW_VERSION+="${PART}"
		
		if [ ${INDEX} -lt ${PERIOD_STOP} ]
		then
			NEW_VERSION+="."
		fi
	done   

	eval $OUTPUT_VAR="'${NEW_VERSION}'"
}

function UUReadGitRevisionNumber
{
	if [ $# != 1 ]
	then
		echo "Usage: UUReadGitRevNumber [Output Variable]"
		exit 1
	fi
	
	local OUTPUT_VAR=$1
	VAR_RESULT=`git rev-list --count HEAD`
    eval $OUTPUT_VAR="'${VAR_RESULT}'"
    
    UUCheckReturnCode $? "readGitRevisionNumber"
}

function UURevertGitChanges
{
	if [ $# != 1 ]
	then
		echo "Usage: UURevertGitChanges [Path to File]"
		exit 1
	fi
	
	local FILE_PATH=$1
	
	echo "Resetting git changes to ${FILE_PATH}"
    git checkout HEAD "${FILE_PATH}"
    
    UUCheckReturnCode $? "revert git changes ${FILE_PATH}"
}

function UURevertSvnChanges
{
	if [ $# != 1 ]
	then
		echo "Usage: UURevertSvnChanges [Path to File]"
		exit 1
	fi
	
	local FILE_PATH=$1
	
	echo "Resetting svn changes to ${FILE_PATH}"
    svn revert -R "${FILE_PATH}"
    
    UUCheckReturnCode $? "revert svn changes ${FILE_PATH}"
}

function UURevertChanges
{
	if [ $# != 1 ]
	then
		echo "Usage: UURevertChanges [Path to File]"
		exit 1
	fi
	
	local FILE_PATH=$1
	
	UUIsGitRepo LOCAL_IS_GIT_REPO
	UUIsSvnRepo LOCAL_IS_SVN_REPO
	
	if [ ${LOCAL_IS_GIT_REPO} == 1 ]
	then
		UURevertGitChanges "${FILE_PATH}"
	fi
	
	if [ ${LOCAL_IS_SVN_REPO} == 1 ]
	then
		UURevertSvnChanges "${FILE_PATH}"
	fi
}

function UUDeleteFile
{
	if [ $# != 1 ]
	then
		echo "Usage: UUDeleteFile [Source]"
		exit 1
	fi
	
	local SOURCE=$1
	
	echo "Deleting ${SOURCE}"
	rm -rf "${SOURCE}"

	UUCheckReturnCode $? "delete file failed"
}

function UUCopyFile
{
	if [ $# != 2 ]
	then
		echo "Usage: UUCopyFile [Source] [Dest]"
		exit 1
	fi
	
	local SOURCE=$1
	local DEST=$2
	
#	echo "Deleting ${DEST}"
#	rm -rf "${DEST}"

	UUDebugLog "Copying ${SOURCE} to ${DEST}"
	cp -r "${SOURCE}" "${DEST}"

	UUCheckReturnCode $? "copy file failed"
}

function UUCreateFolder
{
	if [ $# != 1 ]
	then
		echo "Usage: UUCreateFolder [FolderName]"
		exit 1
	fi
	
	local FOLDER_NAME=$1
	
	UUDebugLog "Creating folder ${FOLDER_NAME}"
	mkdir -p "${FOLDER_NAME}"

	UUCheckReturnCode $? "creating folder"
}

function UUExtractFileName
{
	if [ $# != 2 ]
	then
		echo "Usage: UUExtractFileName [Full Path] [Output Variable]"
		exit 1
	fi
	
	local FULL_PATH=$1
	local OUTPUT_VAR=$2
	local FILE_NAME=$(basename "$FULL_PATH")

    eval $OUTPUT_VAR="'${FILE_NAME}'"
    
    UUCheckReturnCode $? "UUExtractFileName"
}

function UUExtractFileNameNoExtension
{
	if [ $# != 2 ]
	then
		echo "Usage: UUExtractFileNameNoExtension [Full Path] [Output Variable]"
		exit 1
	fi
	
	local FULL_PATH=$1
	local OUTPUT_VAR=$2
	
	local FILE_WITH_EXTENSION=$(basename "$FULL_PATH")
	local FILE_WITHOUT_EXTENSION="${FILE_WITH_EXTENSION%.*}"
	
    eval $OUTPUT_VAR="'${FILE_WITHOUT_EXTENSION}'"
    
    UUCheckReturnCode $? "UUExtractFileNameNoExtension"
}

function UUExtractFileExtension
{
	if [ $# != 2 ]
	then
		echo "Usage: UUExtractFileExtension [Full Path] [Output Variable]"
		exit 1
	fi
	
	local FULL_PATH=$1
	local OUTPUT_VAR=$2
	local FILE_EXTENSION="${FULL_PATH##*.}"

    eval $OUTPUT_VAR="'${FILE_EXTENSION}'"
    
    UUCheckReturnCode $? "UUExtractFileExtension"
}

function UUExtractFolder
{
	if [ $# != 2 ]
	then
		echo "Usage: UUExtractFolder [Full Path] [Output Variable]"
		exit 1
	fi
	
	local FULL_PATH=$1
	local OUTPUT_VAR=$2
	local DIR=$(dirname "${FULL_PATH}")

    eval $OUTPUT_VAR="'${DIR}'"
    
    UUCheckReturnCode $? "UUExtractFolder"
}

function UUFindFolder
{
	if [ $# != 3 ]
	then
		echo "Usage: UUFindFolder [Root Search Path] [VarName] [Output Variable]"
		exit 1
	fi
	
	local ROOT_SEARCH_PATH=$1
	local VAR_NAME=$2
	local OUTPUT_VAR=$3
	
	local RESULT=`find "${ROOT_SEARCH_PATH}" -type d -name "${VAR_NAME}" | head -n 1`
	
	eval $OUTPUT_VAR="'${RESULT}'"
	
	UUCheckReturnCode $? "UUFindFolder"
}

function UUZipFolder
{
	if [ $# != 2 ]
	then
		echo "Usage: UUZipFolder [Source] [Dest]"
		exit 1
	fi
	
	local SOURCE=$1
	local DEST=$2
	
	CWD=$(pwd)
	cd "${SOURCE}"
	cd ..
	
	local FOLDER=$(basename "$SOURCE")
	
	UUDebugLog "CWD: $(pwd)"
	UUDebugLog "FOLDER: ${FOLDER}"
	
	zip -r "${DEST}" "${FOLDER}"
	UUCheckReturnCode $? "UUZipFolder"

	cd "${CWD}"
}

function UUEscapeChars 
{
	if [ $# != 1 ]
    then
        echo "Usage: UUEscapeChars [input]"
        exit 1
    fi
    
    local INPUT=$1
	INPUT="${INPUT/\&/\\\&}"
    echo ${INPUT}
}

function UUReplaceStringInFile
{
	if [ $# != 3 ]
    then
        echo "Usage: UUReplaceStringInFile [file] [to replace] [replacement]"
        exit 1
    fi
    
    local FILE=$1
    local TO_REPLAC=$2
    local REPLACEMENT=$3
    
    sed -i "" -e "s/${TO_REPLAC}/${REPLACEMENT}/g" "${FILE}"
}
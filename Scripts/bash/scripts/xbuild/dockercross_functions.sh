#!/bin/bash

declare -a dockercross_images 	# must be declared

DOCKERCROSS_NAMESPACE="dockcross"
DOCKERCROSS_MAKEFILE_PREFIX_PATH=${VCS_ROOT_DIR}/Docker/Config/compile/

function dockercross_get_images_list 
{
	curl -sS https://raw.githubusercontent.com/dockcross/dockcross/master/Makefile -o ${DOCKERCROSS_MAKEFILE_PREFIX_PATH}/dockcross-Makefile
	for image in $(make -f ${DOCKERCROSS_MAKEFILE_PREFIX_PATH}/dockcross-Makefile display_images); do
		dockercross_images+=([$image]="${DOCKERCROSS_NAMESPACE}/${image} ")
	done
	print_header ${LABEL:-"dockercross_get_images_list"}
	print_array "${dockercross_images[@]}"
	print_separator
}

function dockercross_install_base 
{
	CROSS_COMPILER_IMAGE_NAME=${1}
	print_header ${LABEL:-"dockercross_install_base"}
	print_separator

	curl -sS https://raw.githubusercontent.com/dockcross/dockcross/master/Makefile -o ${DOCKERCROSS_MAKEFILE_PREFIX_PATH}/dockcross-Makefile
	for image in $(make -f ${DOCKERCROSS_MAKEFILE_PREFIX_PATH}/dockcross-Makefile display_images); do
		if [[ "$CROSS_COMPILER_IMAGE_NAME" == "${DOCKERCROSS_NAMESPACE}/$image" ]]; then
			print_prefix_line " *** [Pull] *** registry pulled >>> dockcross/$image"
			docker run --rm ${CROSS_COMPILER_IMAGE_NAME} > ${DOCKERCROSS_MAKEFILE_PREFIX_PATH}/dockcross
			chmod +x ${DOCKERCROSS_MAKEFILE_PREFIX_PATH}/dockcross
			mv ${DOCKERCROSS_MAKEFILE_PREFIX_PATH}/dockcross ${SNIPERKIT_BIN_DIR}
		else
			print_prefix_line " *** [Skip] *** dockcross/$image as user input provided is \"${CROSS_COMPILER_IMAGE_NAME}\")"					
		fi
	done

	print_separator

}

function dockercross_deploy_all 
{

	curl -sS https://raw.githubusercontent.com/dockcross/dockcross/master/Makefile -o ${DOCKERCROSS_MAKEFILE_PREFIX_PATH}/dockcross-Makefile
	for image in $(make -f ${DOCKERCROSS_MAKEFILE_PREFIX_PATH}/dockcross-Makefile display_images); do
	  if [[ $(docker images -q dockcross/$image) == "" ]]; then
		print_prefix_line "${SNIPERKIT_BIN_DIR}/dockcross-$image  *** [skipping] *** image not found locally"					
	    continue
	  fi
	  print_prefix_line "${SNIPERKIT_BIN_DIR}/dockcross-$image  *** [ok] ***"					
	  docker run dockcross/$image > ${SNIPERKIT_BIN_DIR}/dockcross-$image && \
	  chmod u+x ${SNIPERKIT_BIN_DIR}/dockcross-$image
	done
}


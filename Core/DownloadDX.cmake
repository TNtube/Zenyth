# DXC Download Utility
# Place this in a file named 'DownloadDXC.cmake'

include(FetchContent)

function(download_dxc TARGET_DIR)
    set(DXC_DOWNLOAD_DIR "${TARGET_DIR}/dxc")


    set(DXC_DOWNLOAD_URL "https://github.com/microsoft/DirectXShaderCompiler/releases/download/v1.8.2502/dxc_2025_02_20.zip")
    set(DXC_VERSION "2025_02_20")

    # Create the target directory if it doesn't exist
    if(NOT EXISTS "${DXC_DOWNLOAD_DIR}")
        file(MAKE_DIRECTORY "${DXC_DOWNLOAD_DIR}")
    endif()

    # Skip download if we already have it
    if(EXISTS "${DXC_DOWNLOAD_DIR}/bin/x64/dxc.exe")
        message(STATUS "DXC already downloaded at ${DXC_DOWNLOAD_DIR}")
        return()
    endif()

    # Download the ZIP file
    message(STATUS "Downloading DXC from ${DXC_DOWNLOAD_URL}...")
    file(DOWNLOAD "${DXC_DOWNLOAD_URL}" "${CMAKE_BINARY_DIR}/dxc_latest.zip" STATUS DOWNLOAD_STATUS)
    list(GET DOWNLOAD_STATUS 0 STATUS_CODE)

    if(NOT STATUS_CODE EQUAL 0)
        list(GET DOWNLOAD_STATUS 1 ERROR_MESSAGE)
        message(WARNING "Failed to download DXC: ${ERROR_MESSAGE}")
        return()
    endif()

    # Extract the ZIP file
    message(STATUS "Extracting DXC to ${DXC_DOWNLOAD_DIR}...")
    file(ARCHIVE_EXTRACT
            INPUT "${CMAKE_BINARY_DIR}/dxc_latest.zip"
            DESTINATION "${DXC_DOWNLOAD_DIR}"
    )

    # Check if extraction was successful
    if(EXISTS "${DXC_DOWNLOAD_DIR}/bin/x64/dxc.exe")
        message(STATUS "DXC successfully downloaded and extracted to ${DXC_DOWNLOAD_DIR}")
    else()
        message(WARNING "Failed to extract DXC properly. Check the downloaded archive.")
    endif()

    # Clean up the ZIP file
    file(REMOVE "${CMAKE_BINARY_DIR}/dxc_latest.zip")
endfunction()

# Usage example (uncomment to use directly):
# download_dxc("${CMAKE_CURRENT_SOURCE_DIR}/Deps")
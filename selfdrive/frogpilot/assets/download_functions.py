import os
import shutil

LOCAL_PATH = "/path/to/local/resources/"

def get_repository_url():
    return LOCAL_PATH

def download_file(cancel_param, destination, temp_destination, progress_param, file_name, download_param, params_memory):
    try:
        local_file_path = os.path.join(get_repository_url(), file_name)

        if not os.path.isfile(local_file_path):
            print(f"File not found locally: {local_file_path}")
            handle_error(temp_destination, "File not found locally", "File not found", download_param, progress_param, params_memory)
            return

        os.makedirs(os.path.dirname(destination), exist_ok=True)
        shutil.copy(local_file_path, destination)
        params_memory.put(progress_param, "100%")
    except Exception as e:
        handle_request_error(e, temp_destination, download_param, progress_param, params_memory)

def handle_error(destination, error_message, error, download_param, progress_param, params_memory):
    if destination:
        delete_file(destination)

    if download_param:
        params_memory.remove(download_param)

    if progress_param and "404" not in error_message:
        print(f"Error occurred: {error}")
        params_memory.put(progress_param, error_message)

def handle_request_error(error, destination, download_param, progress_param, params_memory):
    print(f"An error occurred: {error}")
    handle_error(destination, "Unexpected error", error, download_param, progress_param, params_memory)

def verify_download(file_path, temp_file_path, file_name, initial_download=True):
    local_file_path = os.path.join(get_repository_url(), file_name)

    if not os.path.isfile(local_file_path):
        print(f"Error fetching local file: {local_file_path}")
        return False if initial_download else True

    if not os.path.isfile(temp_file_path):
        print(f"File not found: {temp_file_path}")
        return False

    try:
        local_file_size = os.path.getsize(local_file_path)
        if local_file_size != os.path.getsize(temp_file_path):
            print(f"File size mismatch for {temp_file_path}")
            return False
    except Exception as e:
        print(f"An unexpected error occurred while verifying the {temp_file_path} download: {e}")
        return False

    os.rename(temp_file_path, file_path)
    return True

def delete_file(file_path):
    try:
        if os.path.isfile(file_path):
            os.remove(file_path)
            print(f"Deleted file: {file_path}")
    except Exception as e:
        print(f"Failed to delete file {file_path}: {e}")

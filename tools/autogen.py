import os

def handle_h_file(path: str):
    with open(path, 'r') as file:
        first_line = file.read().readline()
        if first_line.rstrip('\n').strip() == "#pragma once":
            rest_of_file = file.readlines()
            file.truncate(0)
            file.write("#pragma once // (c) 2020 Lukas Brunner")
            for line in rest_of_file:
                file.write(line)

FILE_HANDLERS = {
    '.test': handle_h_file
}

def main():
    for folder, folder_names, file_names in os.walk("./"):
        for file_name in file_names:
            name, ext = os.path.splitext(file_name)
            handler = FILE_HANDLERS.get(ext, None)
            if handler is not None:
                handler(os.path.join(dirpath, f))

if __name__ == '__main__':
    main()
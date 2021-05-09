import os

def handle_h_file(path: str):
    with open(path, 'r+') as file:
        file.seek(0)
        first_line = '\n'
        while first_line == '\n':
            first_line = file.readline()

        if first_line.rstrip('\n').strip() == "#pragma once":
            print("Updating file header in {}".format(path))
            print('Valid match:\n{}'.format(first_line))
            rest_of_file = file.readlines()
            file.seek(0)
            file.truncate(0)
            file.write("#pragma once // (c) 2021 Lukas Brunner\n")
            for line in rest_of_file:
                file.write(line)

FILE_HANDLERS = {
    '.h': handle_h_file
}

def main():
    for folder, folder_names, file_names in os.walk("./"):
        for file_name in file_names:

            name, ext = os.path.splitext(file_name)
            handler = FILE_HANDLERS.get(ext, None)
            if handler is not None:
                file_path = os.path.join(folder, file_name)
                handler(file_path)

if __name__ == '__main__':
    main()
    exit(0)
# compilerForIcs
A C program that converts SUTD's MyPortal calendar into a standard .ics file. This calendar can then be imported into sites such as Google Calender.

## Step 1: Prerequisites

1. First, ensure that your system has gcc installed. 

**Windows + VS Code:** 
https://code.visualstudio.com/docs/cpp/config-mingw

**Mac:** 
Run the code below in your terminal to install `make` and gcc
```
xcode-select --install
```


**Verify that GCC is installed by running in your terminal:**

`gcc --version`

2. Clone the program from GitHub:

`git clone https://github.com/killerdolp/compilerForIcs.git`

## Step 2: Retrieve your calendar from MyPortal

1. Login to MyPortal. Under "My Record", click on "My Weekly Schedule".

<img width="587" height="260" alt="image" src="https://github.com/user-attachments/assets/73866587-a9d7-4fd2-ad7c-6b231789883d" />

2. Click on "List View".

<img width="807" height="408" alt="image" src="https://github.com/user-attachments/assets/16aefca1-00f1-4273-bc3d-6b9b61170d16" />


3. Right click and Save As Webpage, Single File.

<img width="322" height="67" alt="image" src="https://github.com/user-attachments/assets/ec4981ad-c04d-4da5-94a5-5faba209bf4f" />

4. Rename the file extension from `.mhtml` to `.html`

<img width="207" height="30" alt="image" src="https://github.com/user-attachments/assets/b36c4c62-78b1-445f-898b-562a53b14a8b" />

## Step 3: Compilation

OPTIONAL: Clean existing binaries/temporary files using:

`make clean`

Compile the program using:

`make`

or:

`gcc fsm.c main.c ics_writer.c -o main -ansi -pedantic -Wall -Werror`

## Step 4: Running

Run the program with the following command:

`./main`

If you want to use a specific HTML file, enter its exact filename when running:

`./main <filename>.html`

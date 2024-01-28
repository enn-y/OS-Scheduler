# G52OSC Operating Systems and Concurrency (Group - PGB02)
## Project Description

In this coursework, the group has implemented 3 schedulers in C programming language. The 3 schedulers are: First Come First Serve (FCFS), Round Robin (RR), and Multi-Level Feedback Queue (MLFQ). All 3 schedulers are combined into ONE code file. The users of the program are given the option to choose which scheduler they want to run, then also enter the following: Arrival Time, Burst Time, and Time Quantum for RR and MLFQ. Upon successful entry of values into the scheduler algorithm, the program will display: the initial queue, the process start and end times, gantt chart, execution and idle blocks, the scheduling calculation information, and the scheduling calculation performance. 

## Requirements 

Any IDE software that supports compiling and editing of C language (e.g. VSCode or CodeBlocks).

## How to Run
To execute the C program using an Integrated Development Environment (IDE) or a command terminal, follow the steps laid out below:

For IDEs including VSCode, CodeBlocks, and CLion:
1)	Launch your preferred C language IDE. For VSCode, ensure that the C/C++ extension by Microsoft is installed for code compilation and debugging. 
2)	Navigate to the project or folder location and open the main.c file which contains the source code.
3)	For dedicated IDEs:
    a.	For VSCode:
        i.	Use the integrated terminal to compile the code by typing `gcc -o program main.c`
        ii.	Run the compiled program by typing `./program` in the terminal.
    b.	For CLion:
        i.	Click on the ‘Run’ button on the top right corner to execute the program.
4)	The first prompt will require entering the choice of algorithm from First Come, First Served, Round Robin, or Multi-Level Feedback Queue.
5)	Succeeding prompts will require the number of tasks processes, with the arrival and burst times for each of the given processes. Round Robin and Multi-Level Feedback Queue     algorithms will require Time Quantum.
6)	Upon successful inputs, the program will output the initial process queue, start and end times of each process, the Gantt chart illustrating the execution timeline, detailed scheduling calculations, and the performance metrics including average response, waiting, and turnaround times, alongside the throughput.

For command terminal on Unix-like systems:
1)	Open the terminal.
2)	Similarly, as before, navigate to the directory containing the `main.c` file.
3)	Then compile the file with `gcc -o program main.c`.
4)	Type `./program` to execute the program and provide the inputs as described earlier.
5)	The results will be displayed directly in the terminal.

For command prompt on Windows:
1)	Open the Command Prompt
2)	Similarly, as before, redirect to the directory containing the `main.c` file:
3)	Compile the file using the command `gcc -o program main.c`.
4)	Run the program with `program.exe` and likewise provide the necessary inputs prompted by the program.
5)	The results will be displayed in the CMD window.


## Contributors 
- Lua Chong En hcycl5@nottingham.edu.my
- Joseph Emmanuel Chay Huan Qin hcyjc11@nottingham.edu.my
- Jonathan James Rawding hfyjk6@nottingham.edu.my
- Jie Zhan Keh hfyjk6@nottingham.edu.my

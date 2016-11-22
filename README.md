Lisa Fan & Tyler Frasca
lfan01   & tfrasc01

Process Data Structure:
	-We created a data structure called Process to keep track of processes.
	
	-Each Process has a pid, arrival time, service time, priority (as a nice value), and runtime
	
	-Other values related to Processes, such as the Linux weight, virtual runtime, and timeslice are not part of the Process struct. We decided on this design, since these values are specific to the CFS algorithm, and would likely be tracked by the kernel, and not the Process itself.
	
	-When we read in the processes from the input file, we store them in an array of Processes, and sort that array by arrival time. If some processes have the same arrival time, we also sort by priority. This is to simplify things for the scheduler algorithms, as well as to better simulate actual events.


Traditional Algorithm:
	-We chose the FCFS (First Come First Serve) algorithm for our traditional scheduler, because we believed this would be the easiest to implement. This algorithm only requires you to keep track of arrival time, and each process runs to completion once it starts processing. The other algorithms require you to compute and keep track of other information additional to arrival time, such as service time, current concurrent runtime, and time remaining (service time - runtime).
	
	-Our ready queue is implemented as an array of Processes, and operates similar to the Consumer/Producer problem. One pointer keeps track of the next empty index in the queue, and another keeps track of the next Process to "run". If the second pointer catches up to the first pointer, it waits until another Process arrives and enters the ready queue.
	
	-The priority of each process is stored in the struct, but not used by the scheduler.
	
	-At each timestep, the following things happen:
		1. If new processes have arrived, they are added to the ready queue.
	
		2. If no process is currently running, one is taken off the ready queue.
			-If the ready queue is empty, the scheduler prints an idle message and continues to the next timestep.
	
		3. The runtime on the current process, and the CPU usage time are incremented.
	
		4. If the current process has run for as long as its service time, the sum of the TAT and NTAT are updated, and we say that no process is currently running.
	
	-The above loop continues until all the processes have finished running. We check this by keeping track of the total number of processes in the input file, and the number of processes that have finished running.
	
	-After the loop, we print the total CPU usage percentage, calculated by dividing the CPU usage time by end time, and the average TAT and NTAT, calculated by dividing the sums by the number of processes.


Linux CFS:
	-Our ready queue is implemented as a red-black tree (see Extra Credit)
	
	-At each timestep, the following things happen:
		1. If new processes have arrived, they are added to the ready queue, and their weights are added to the sum of weights for all Processes on the ready queue (used to calculate timeslice)
	
		2. If no process is currently running, one is taken off the ready queue. (See below for how we pick which one)
			-If the ready queue is empty, the scheduler prints an idle message and continues to the next timestep.
			-If there is a process on the ready queue, its timeslice is calculated.
	
		3. The runtime on the current process, and the CPU usage time are incremented. Two separate runtimes exist for the process: the total runtime for the process, and the current runtime since the process most recently got CPU access.
	
		4. If the current process has run for as long as its service time, the sum of the TAT and NTAT are updated, the weight of the process is taken off the sum of weights in the ready queue, and we say that no process is currently running.
	
		5. If the current process has run for as long as its timeslice, its virtual runtime is updated, the process is re-inserted into the ready queue, and we say that no process is currently running.
	
	-The above loop continues until all the processes have finished running. We check this by keeping track of the total number of processes in the input file, and the number of processes that have finished running.
	
	-After the loop, we print the total CPU usage percentage, calculated by dividing the CPU usage time by end time, and the average TAT and NTAT, calculated by dividing the sums by the number of processes.
	
	-Apart from increments and simple sums, there are two significant calculations in the CFS algorithm:
		-Timeslice: The equation for calculating timeslice is target latency * (process weight / sum of ready weights). We decided to store timeslice as an integer, and not a float, since the timeslice is used to calculate how many timesteps a process is to run for. However, this means that there are cases when the timeslice is 0 using the above equation, particularly when the target latency is small, the process weight is small, and/or the sum of weights is large. In order to avoid this, we decided to always round up the calculation by casting the calculation to a float, using the ceil() function to round up, and then casting back into an int.
	
		-Virtual Runtime: We calculate the virtual runtime as follows: process total runtime * (weight of nice value=0 / process weight). We decided to store the virtual runtime as a float, since otherwise any process with a nice value less than 0 would have a virtual runtime of 0.
	
	-When taking a process off the ready queue, we choose the process with the smallest virtual runtime. Since our ready queue is a red black tree with virtual runtime as the keys (see below for more details), this meant finding the leftmost node in the red black tree. Because all processes start with a virtual runtime of zero, our CFS algorithm chooses amongst these processes in the following way:
		1. If multiple processes arrive at different times while the CPU is busy, the process that arrived first gets chosen next
	
		2. If multiple processes arrive at the same time while the CPU is busy, the process with the higher priority gets chosen next
		This ordering is established both by the compare function for the red-black tree, and the compare function that sorts the processes from the input file
	
	-Target Latency: We chose our target latency to be 5 because we imagined that most input files would have a small number of processes (minimum 5, maximum perhaps 100). Because these processes would all have different arrival and service times, they would not all be in the ready queue at the same time. Since the target latency is meant to represent the time in which all processes on the ready queue run at least once, we believed that assuming there would be about 5 processes in the ready queue would be a fair assumption. Furthermore, since this is a simulation, we assumed that the service times for the processes would be relatively small. By choosing a small target latency, we would have more of a chance of seeing processes swap out of the CPU, so that we could test that our scheduler was working.


Test Files:
	-When we validate the input file, we check the following things:
		-each line has 4 integer values
		-the pid and arrival time are at least 0
		-the service time is at least 1
		-the priority is between -20 and 19
		-the pids are unique

	input1.txt:
		-There are 13 processes in this file.
		-The first 5 processes test the edge case of multiple new processes arriving at the same time or close to one another. In CFS, P2-5 arrive while P1 is running. P2 should run before P3-5, since it arrived one timestep earlier than the others. After P2, P5 P3 P4 should run in that order, based on their priorities.
		-P6-13 are processes that arrive around the same time with a large variety of service time and priorities. We see the processes executing for the first time in the order explained above. We also see that P11 runs frequently in earlier timesteps, because this process has the highest priority. We see P12 running frequently and finishing last, because this process has the lowest priority.
		-Running this file with the FCFS scheduler produced the following results:
			CPU usage : 100.00 %
			Average TAT : 50.77
			Average normalized TAT : 5.78
		-Running this file with the CFS scheduler produced the following results:
			CPU usage : 100.00 %
			Average TAT : 79.46
			Average normalized TAT : 12.84
		-For this dataset, FCFS performs better than CFS. This is because CFS takes priority into account, whereas FCFS does not. Because our highest priority process has a long service time, other shorter processes must wait longer to complete.

	input2.txt:
		-There are 10 processes in this file.
		-The file was generated using our processes.c file (see below).
		-Running this file with FCFS scheduler produced the following results:
			CPU usage : 99.05 %
			Average TAT : 34.30
			Average normalized TAT : 3.99
		-Running this file with the CFS scheduler produced the following results:
			CPU usage : 99.05 %
			Average TAT : 44.30
			Average normalized TAT : 4.01

	processes.c:
		-In addition to our two input files, we also wrote a program called processes.c, that generates random processes.
		-To compile: make processes
		-To run: ./processes <number of processes> <output file name>
		-The pids of the processes will range from 0 to the number of processes specified, but they will appear in random order. The arrival time is randomized between 0 and 49, the service time between 1 and 20, and the priority between -20 and 19.


Extra Credit:
	-We implemented our ready queue for the CFS scheduler with a red-black tree. 
	
	-The source code for the red-black tree was taken from http://web.mit.edu/~emin/Desktop/ref_to_emin/www.old/source_code/red_black_tree/index.html, and all of the code taken is stored in the rb_tree directory.
	
	-Our ready queue stores virtual runtime as the key, and a pointer to its associated Process as the value for each node.
	
	-We wrote a compare function for sorting the red-black tree, which stores the node with the lowest virtual runtime in the leftmost position, and the node with the highest virtual runtime in the rightmost position. 
	
	-We were also required to declare a function to be passed to the remove node function. Because our values are pointers to Processes which may be used again, we do not need to free any memory, and so this function is blank.

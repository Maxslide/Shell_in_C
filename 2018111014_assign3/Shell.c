#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <error.h>
#include <readline/readline.h>
#include <readline/history.h>
#define buf 1024
#define RED "\x1b[31m"
#define GREEN "\x1b[32m"
#define YELLOW "\x1b[33m"
#define BLUE "\x1b[34m"
#define MAGENTA "\x1b[35m"
#define CYAN "\x1b[36m"
#define RESET "\x1b[0m"
char home[buf];
char cwd[buf];
int command = 0;
char inp[buf];
char tilda = '~';
char user[buf];
char host[buf];
char relinp[buf];
int alphaflag[200];
int redirflag = 0;
int jobs[100];
int jo = 0;
//char flag[10][buf];
char prompt[buf] = "";
char printarr[buf];
// ------- Function Declaration -----
void process_terminated(int sig);
int sys_com(char *sys_c, char *name);
int get_com_inp();
int pwd();
int cd(char *inpstr);
void CtrlC(int sig);
void CtrlZ(int sig);
int HOME();
int cronjob();
char *ltrim(char *str, const char *seps);
char *rtrim(char *str, const char *seps);
char *trim(char *str, const char *seps);
int CWD();
int ls(char dir_n[buf], long int tok_len);
int clear();
int tokenise();
int free_flag();
int echo(char *echpr);
int ls_l(char *fname);
int pinfo(char *pid);
int showhis(int n);
int envar();
int unenvar();
int redirect();
int jobsCom();
int killjob();
int overkill();
int quit();
int fg();
// End Function Declaration
char *ltrim(char *str, const char *seps)
{
	size_t totrim;
	if (seps == NULL)
	{
		seps = "\t\n\v\f\r ";
	}
	totrim = strspn(str, seps);
	if (totrim > 0)
	{
		size_t len = strlen(str);
		if (totrim == len)
		{
			str[0] = '\0';
		}
		else
		{
			memmove(str, str + totrim, len + 1 - totrim);
		}
	}
	return str;
}
char *rtrim(char *str, const char *seps)
{
	int i;
	if (seps == NULL)
	{
		seps = "\t\n\v\f\r ";
	}
	i = strlen(str) - 1;
	while (i >= 0 && strchr(seps, str[i]) != NULL)
	{
		str[i] = '\0';
		i--;
	}
	return str;
}
char *trim(char *str, const char *seps)
{
	return ltrim(rtrim(str, seps), seps);
}
int tokenise(char *inp, char *delim, char flag[10][buf])
{
	char *token = strtok(inp, delim);
	int i = 0;
	for (int j = 0; j < 10; j++)
	{
		for (int k = 0; k < buf; k++)
		{
			flag[j][k] = '\0';
		}
	}
	//printf("in tokenise");
	while (token != NULL)
	{
		//flag[i] = (char *) malloc(buf * sizeof(char));
		//printf("after memory allocation");
		strcpy(flag[i], token);
		token = strtok(NULL, delim);
		i++;
	}
	return i;
}
int free_flag()
{
	for (int i = 0; i < 200; i++)
	{
		// printf("%d\n", i);
		alphaflag[i] = 0;
	}
}
int cronjob(char *inpu)
{
	char flag[10][buf];
	int n = tokenise(inpu, " ", flag);
	if (n != 6)
	{
		printf("incorrect number of arguments, cannot carry out cronjob\n");
	}
	else
	{
		int rep = atoi(flag[3]);
		int ter = atoi(flag[5]);
		for (int i = 0; i < ter / rep; i++)
		{
			sleep(rep);
			get_com_inp(flag[1]);
		}
	}
}
int get_com_inp(char *abc)
{
	char pipestr[buf];
	char abccpy[buf];
	strcpy(abccpy, abc);
	int i = 0;
	while (abccpy[i] != '\0')
	{
		if (abccpy[i] == '>' || abccpy[i] == '<')
		{
			redirect(abccpy);
			return 0;
		}
		i++;
	}

	char tokens[buf];
	for (int i = 0; i < buf; i++)
	{
		tokens[i] = '\0';
	}
	char comd[buf];
	i = 0;
	//printf("%s\n", inp);
	while (abc[i] != ' ' && abc[i] != '\0')
	{
		comd[i] = abc[i];
		i++;
	}
	comd[i] = '\0';
	int j = 0;
	while (abc[i] != '\0')
	{
		tokens[j] = abc[i];
		j++;
		i++;
	}
	trim(tokens, NULL);
	//printf("%s", comd);
	//printf("%d\n",strcmp(comd, "pinfo"));
	if (strcmp(comd, "pwd") == 0)
	{
		pwd();
	}
	else if (strcmp(comd, "bg") == 0)
	{
		trim(tokens, " ");
		int num = atoi(tokens);
		killjob(num, 18); // 18 is hte signal number or the signal SIGCON
	}
	else if (strcmp(comd, "fg") == 0)
	{
		trim(tokens, " ");
		int num = atoi(tokens);
		fg(num);
	}
	else if (strcmp(comd, "cronjob") == 0)
	{
		cronjob(tokens);
	}
	else if (strcmp(comd, "jobs") == 0)
	{
		int j = 1;
		for (int i = 0; i < jo; i++)
		{
			//printf("[%d] ", j);
			j = jobsCom(jobs[i], j);
		}
	}
	else if (strcmp(comd, "kjob") == 0)
	{
		char inpcpy[buf];
		strcpy(inpcpy, tokens);
		char flag[10][buf];
		tokenise(inpcpy, " ", flag);
		int i = 0;
		int j = 1;
		int num = atoi(flag[0]);
		int signal = atoi(flag[1]);
		killjob(num, signal);
	}
	else if (strcmp(comd, "overkill") == 0)
	{
		overkill();
	}
	else if (strcmp(comd, "cd") == 0)
	{
		cd(tokens);
	}
	else if (strcmp(comd, "quit") == 0)
	{
		exit(0);
	}
	else if (strcmp(comd, "ls") == 0)
	{
		ls(tokens, strlen(tokens));
	}
	else if (strcmp(comd, "clear") == 0 || strcmp(comd, "c") == 0)
	{
		clear();
	}
	else if (strcmp(comd, "echo") == 0)
	{
		echo(tokens);
	}
	else if (strcmp(comd, "pinfo") == 0)
	{
		if (tokens[0] == '\0')
		{
			//printf("here\n");
			char curpid[buf];
			snprintf(curpid, buf, "%d", getpid());
			pinfo(curpid);
		}
		else
		{
			//printf("%s", tokens);
			pinfo(tokens);
		}
	}
	else if (strcmp(comd, "setenv") == 0)
	{
		envar(tokens);
	}
	else if (strcmp(comd, "unsetenv") == 0)
	{
		unenvar(tokens);
	}
	else
	{
		sys_com(comd, tokens);
	}
	return 0;
}
int overkill()
{
	for (int i = 1; i <= jo; i++)
	{
		killjob(i, 9);
	}
}
int redirect(char *abc)
{
	char red[buf];
	int i = 0;
	int j = 0;
	int status;
	while (abc[i] != '\0')
	{
		if (abc[i] != '<')
		{
			red[j++] = abc[i];
		}
		i++;
	}
	red[j] = '\0';
	i = 0;
	int append = 0;
	while (red[i] != '\0')
	{
		if (red[i] == '>')
			append++;
		i++;
	}
	if (append > 0)
	{
		char flag[10][buf];
		int out;
		tokenise(red, ">", flag);
		trim(flag[0], " ");
		trim(flag[1], " ");
		char fag[10][buf];
		char *argv[buf];
		tokenise(flag[0], " ", fag);
		i = 0;
		while (fag[i][0] != 0)
		{
			argv[i] = fag[i];
			i++;
		}
		argv[i] = NULL;
		pid_t child_pid, tpid;
		redirflag = 1;
		child_pid = fork();
		if (child_pid == 0)
		{
			redirflag = 1;
			if (append == 1)
				out = open(flag[1], O_WRONLY | O_CREAT, 0666); // Should also be symbolic values for access rights
			else
				out = open(flag[1], O_WRONLY | O_CREAT | O_APPEND, 0666); // Should also be symbolic values for access rights
			//printf("file created");
			dup2(out, STDOUT_FILENO);
			close(out);
			//printf("here3\n");
			//signal(SIGCHLD,process_terminated);
			execvp(argv[0], argv);
			//printf("Unknown command\n");
			close(1);
			exit(0);
		}
		else
		{

			tpid = waitpid(child_pid, &status, 0);
		}
	}
	else
	{
		char *argv[buf];
		char flag[10][buf];
		int out;
		strcpy(red, abc);
		int n = tokenise(red, "<", flag);
		trim(flag[0], " ");
		trim(flag[1], " ");
		for (int i = 0; i < n; i++)
		{
			argv[i] = flag[i];
		}
		argv[i] = NULL;
		pid_t child_pid, tpid;
		child_pid = fork();

		if (child_pid == 0)
		{
			execvp(argv[0], argv);
			exit(0);
		}
		else
		{
			tpid = waitpid(child_pid, &status, 0);
		}
	}
	redirflag = 0;
}
int unenvar(char *env)
{
	if (env[0] == '\0')
	{
		printf("error in commmand line arguments");
	}
	else
	{
		unsetenv(env);
	}
}
int envar(char *env)
{
	char flag[10][buf];
	int n = tokenise(env, " ", flag);
	if (n != 1)
	{
		printf("error in the command arguments");
	}
	else
	{
		char var[buf];
		char val[buf];
		for (int i = 0; i < buf; i++)
		{
			val[i] = '\0';
		}
		int i = 0;
		while (flag[0][i] != '[')
		{
			var[i] = flag[0][i];
			i++;
		}
		var[i] = '\0';
		i++;
		int j = 0;
		while (flag[0][i] != ']')
		{
			val[j] = flag[0][i];
			i++;
			j++;
		}
		val[j] = '\0';
		//printf("%s %s", var, val);
		setenv(var, val, 1);
	}
}
int echo(char *echpr)
{
	/*char flag[10][buf];
	  tokenise(echpr, " ", flag);
	  int i = 0;
	  while(flag[i][0] != '\0')
	  {
	  printf("%s ", flag[i]);
	  i++;
	  }
	  printf("\n");*/
	printf("%s\n", echpr);
}
int clear()
{
	printf("\033[H\033[J");
}
int pwd()
{
	printf("%s\n", cwd);
}
int cd(char *inpstr)
{
	//error handling
	//printf("\n%s\n%s\n", inpstr, flag[0]);
	if (inpstr[0] == '~')
	{
		//use memove
		memmove(inpstr, inpstr + 1, sizeof(inpstr));
		///printf("%s ", flag[0]);
		strcpy(cwd, home);
		strcat(cwd, inpstr);
		chdir(cwd);
		// use the relinp variable
	}
	else
	{
		chdir(inpstr);
		getcwd(cwd, sizeof(cwd));
		// use the relinp varable.
	}
}
int ls(char token[buf], long int tok_len)
{
	struct dirent *dp;
	DIR *dir;
	int check = 0;
	char temp[buf];
	///printf("%s", dir);
	if (tok_len == 0)
	{
		check = 1;
		dir = opendir(".");
	}
	else
	{
		char flag[10][buf];
		//printf("before tokenise");
		tokenise(token, " ", flag);
		for (int i = 0; i < 10; i++)
		{
			///
			//printf("here");
			if (flag[i][0] == '\0')
				break;
			if (flag[i][0] == '-')
			{
				//printf("\n here \n");
				int j = 0;
				while (flag[i][j] != '\0')
				{
					alphaflag[flag[i][j]] = 1;
					j++;
				}
			}
			else
			{
				check = 2;
				getcwd(temp, sizeof(temp));
				cd(flag[i]);
				//printf("%s\n", temp);
				dir = opendir(".");
			}
		}
	}
	//dp = readdir(dir)
	if (check == 0)
	{
		dir = opendir(".");
	}
	while ((dp = readdir(dir)) != NULL)
	{
		if (alphaflag['a'] == 1)
		{
			if (alphaflag['l'] == 1)
				ls_l(dp->d_name);
			else
				printf("%s\t", dp->d_name);
		}
		else if (alphaflag['a'] == 0)
		{
			if (dp->d_name[0] != '.')
			{
				if (alphaflag['l'] == 1)
				{
					ls_l(dp->d_name);
				}
				else
					printf("%s\t", dp->d_name);
			}
		}
	}
	if (check == 2)
	{
		cd(temp);
	}
	printf("\n");
	closedir(dir);
}
int ls_l(char *fname)
{
	struct stat fileStat;
	if (stat(fname, &fileStat) < 0)
		return 1;
	struct passwd *pws;
	pws = getpwuid(fileStat.st_uid);
	struct group *grp;
	grp = getgrgid(fileStat.st_gid);
	printf((S_ISDIR(fileStat.st_mode)) ? "d" : "-");
	printf((fileStat.st_mode & S_IRUSR) ? "r" : "-");
	printf((fileStat.st_mode & S_IWUSR) ? "w" : "-");
	printf((fileStat.st_mode & S_IXUSR) ? "x" : "-");
	printf((fileStat.st_mode & S_IRGRP) ? "r" : "-");
	printf((fileStat.st_mode & S_IWGRP) ? "w" : "-");
	printf((fileStat.st_mode & S_IXGRP) ? "x" : "-");
	printf((fileStat.st_mode & S_IROTH) ? "r" : "-");
	printf((fileStat.st_mode & S_IWOTH) ? "w" : "-");
	printf((fileStat.st_mode & S_IXOTH) ? "x" : "-");
	printf("%3ld", fileStat.st_nlink);
	printf("%13s", pws->pw_name);
	printf("%13s", grp->gr_name);
	printf("%10ld", fileStat.st_size);
	char time[100];
	strcpy(time, ctime(&fileStat.st_mtime));
	time[strlen(time) - 1] = '\0';
	printf("%20s ", time);
	printf("%10s ", fname);
	printf("\n");
	//printf("%2d", fileStat.st_mode);
}
int CWD()
{
	//printf(GREEN);
	strcat(prompt, GREEN);
	//printf("%s :",user);
	strcat(prompt, user);
	strcat(prompt, " : ");
	if (strcmp(cwd, home) == 0)
	{
		//printf(BLUE);
		strcat(prompt, BLUE);
		strcat(prompt, "~ >");
		strcat(prompt, RESET);
		//printf("%c >", tilda);
		//printf(RESET);
	}
	else if (strstr(home, cwd) != NULL)
	{
		//printf(BLUE);
		strcat(prompt, BLUE);
		strcat(prompt, cwd);
		strcat(prompt, " >");
		strcat(prompt, RESET);
		//printf("%s >", cwd);
		//printf(RESET);
	}
	else if (strstr(home, cwd) == NULL)
	{
		getcwd(cwd, sizeof(cwd));
		strcpy(relinp, cwd);
		//printf(BLUE);
		strcat(prompt, BLUE);
		strcat(prompt, "~/");
		//printf("%c/", tilda);
		char abc[100];
		int i = strlen(home) + 1;
		int j = 0;
		while (relinp[i] != '\0')
		{
			abc[j] = relinp[i];
			i++;
			j++;
		}
		abc[j] = '\0';
		strcat(prompt, abc);
		strcat(prompt, " >");
		strcat(prompt, RESET);
		//printf("%s >", &relinp[strlen(home) + 1]);
		//printf(RESET);
	}
}
int HOME()
{
	strcat(prompt, GREEN);
	strcat(prompt, user);
	strcat(prompt, " : ");
	strcat(prompt, BLUE);
	strcat(prompt, "~ >");
	strcat(prompt, RESET);
	/*printf(GREEN);
	  printf("%s: ",user);
	  printf(BLUE);
	  printf("%c >", tilda);	
	  printf(RESET);*/
}
int killjob(int num, int signal)
{
	int i = 0;
	int j = 1;
	while (jobs[i])
	{
		FILE *statusf;
		char path[100];
		snprintf(path, 50, "/proc/%d/status", jobs[i]);
		statusf = fopen(path, "r");
		if (statusf)
		{
			if (j == num)
			{
				kill(jobs[i], signal);
			}
			j++;
		}
		i++;
	}
}
int jobsCom(int pid, int j)
{
	FILE *statusf;
	char path[50], line[100], *p;
	char name[100];
	char state[100];
	snprintf(path, 50, "/proc/%d/status", pid);
	statusf = fopen(path, "r");
	if (!statusf)
		return j;
	while (fgets(line, 100, statusf))
	{
		if (strncmp(line, "State:", 6) == 0)
		{
			p = line + 7;
			while (isspace(*p))
				++p;
			strcpy(state, p);
		}
		else if (strncmp(line, "Name:", 5) == 0)
		{
			p = line + 6;
			while (isspace(*p))
				++p;
			strcpy(name, p);
		}
	}
	trim(state, NULL);
	trim(name, NULL);
	char run[100];
	if (state[0] != 'T')
		strcpy(run, "Running");
	else
	{
		strcpy(run, "Stopped");
	}

	printf("[%d] %s %s [%d]\n", j, run, name, pid);
	return ++j;
}
int pinfo(char *pid)
{
	FILE *statusf;
	char path[50], line[100], *p;
	snprintf(path, 50, "/proc/%s/status", pid);
	statusf = fopen(path, "r");
	if (!statusf)
		return 1;
	while (fgets(line, 100, statusf))
	{
		if (strncmp(line, "State:", 6) == 0)
		{
			p = line + 7;
			while (isspace(*p))
				++p;
			printf("Process Status : %s", p);
		}
		else if (strncmp(line, "Pid:", 4) == 0)
		{
			p = line + 5;
			while (isspace(*p))
				++p;
			printf("pid : %s", p);
		}
		else if (strncmp(line, "VmSize:", 7) == 0)
		{
			p = line + 8;
			while (isspace(*p))
				++p;
			printf("memory : %s", p);
		}
		else
		{
			continue;
		}
	}
	char exe[50];
	char out[buf];
	snprintf(exe, 50, "/proc/%s/exe", pid);
	readlink(exe, out, buf);
	//printf("%d", strlen(out));
	printf("Executable Path : %s\n", out);
	fclose(statusf);
}
void process_terminated(int sig)
{
	pid_t pid;
	int status;
	pid = waitpid(-1, &status, WNOHANG);
	if (pid > 0)
	{
		printf("\nProcess with Pid %d exited normally.\n ", pid);
	}
}

int sys_com(char *sys_c, char *name)
{

	char *argv[buf];
	int check = 0;
	int status;
	//char *abc = "&";
	//char flag[][]
	//printf("hey%s",name );
	char flag[10][buf];
	tokenise(name, " ", flag);
	if (flag[0][0] == '\0')
	{
		argv[0] = sys_c;
		argv[1] = NULL;
	}
	else
	{
		argv[0] = sys_c;
		/*if(strcmp(flag[0],"&") == 0)
		  {
		  argv[1] = NULL;
		  check = 1;
		  }
		  else
		  {
		  argv[1] = flag[0];
		//argv[2] = abc;
		argv[2] = NULL;
		}*/
		int i = 0;
		while (flag[i][0] != 0)
		{
			if (strcmp(flag[i], "&") == 0)
				check = 1;
			argv[i + 1] = flag[i];
			i++;
		}
		argv[i + 1] = NULL;
	}
	pid_t child_pid, tpid;
	signal(SIGCHLD, process_terminated);
	child_pid = fork();
	if (child_pid == 0)
	{
		execvp(argv[0], argv);
		printf("Unknown command\n");
		exit(0);
	}
	else
	{
		jobs[jo++] = child_pid;
		if (check == 0)
		{
			tpid = waitpid(child_pid, &status, WUNTRACED);
		}
	}
}
int fg(int num)
{
	killjob(num, 19); // 19 = SIGSTOP
	int i = 0;
	int j = 1;
	int status;
	while (jobs[i])
	{
		FILE *statusf;
		char path[100];
		snprintf(path, 50, "/proc/%d/status", jobs[i]);
		statusf = fopen(path, "r");
		if (statusf)
		{
			if (j == num)
			{
				kill(jobs[i], 18);
				waitpid(jobs[i], &status, 0);
			}
			j++;
		}
		i++;
	}
}
int min(int a, int b)
{
	if (a < b)
		return a;
	return b;
}
void CtrlC(int sig)
{
	for (int i = 0; i < jo; i++)
	{
		if (getpid() == jobs[i])
		{
			exit(0);
			break;
		}
	}
}
/*void seg(int sig)
{
	return;
}
*/
void CtrlZ(int sig)
{
	return;
}
/*int add_his(char *inp)
  {	
  FILE *hist;
  hist = fopen("history1.txt", "a+");
  char inpcpy[buf];
  strcpy(inpcpy,inp);
  strcat(inpcpy,"\n");
  fputs(inpcpy,hist);
  fclose(hist);
  }*/
int main()
{
	getcwd(home, sizeof(home));
	strcpy(cwd, home);
	getlogin_r(user, sizeof(user));
	gethostname(host, sizeof(host));
	strcat(user, "@");
	signal(SIGINT, CtrlC);
	signal(SIGTSTP, CtrlZ);
	//signal(SIGSEGV, seg);
	strcat(user, host);
	HOME();
	//FILE *hist_temp, *hist;
	char line[100];
	/*hist = fopen("history.txt", "r+");
	  hist_temp = fopen("historytemp.txt", "w+");
	  int temp;
	  if(hist_temp == NULL)
	  {
	  printf("error in opening the file");
	  }
	  else
	  {
	  while(fgets(line, 100, hist))
	  {
	//printf("print -> %s\n",line);
	fprintf(hist_temp, "%s", line);
	}
	}
	fseek(hist_temp,0,0);
	while(fgets(line, 100, hist_temp))
	{
	//printf("print file -> %s",line);
	}
	fclose(hist);*/
	while (1)
	{
		//fseek(hist_temp,0,0);
		char *inptemp = readline(prompt);
		//add_his(inptemp);
		strcpy(inp, inptemp);
		if (inp[0] != '\0')
		{
			add_history(inp);
		}
		HISTORY_STATE *myhist = history_get_history_state();
		HIST_ENTRY **mylist = history_list();
		int fl = 0;
		/*hist = fopen("history.txt","w+");
		  while(fl < min(21,myhist->length))
		  {
		  fprintf(hist,"%s\n", mylist[fl]->line);
		//printf("%s", mylist[fl]->line);
		fl++;
		}
		while(fl < 21 && fgets(line,100,hist_temp))
		{
		//printf("here");
		fprintf(hist,"%s",line);	
		fl++;
		}*/
		for (int i = 0; i < buf; i++)
			prompt[i] = '\0';
		//printf("%s \n", inptemp);
		//printf("%s\n", inp);
		//printf("%s", inp);
		char fag[10][buf];
		char temp[buf];
		strcpy(temp, inp);
		int a = tokenise(temp, "|", fag);
		int i = 0;
		int stdoutcpy = dup(1);
		int stdincpy = dup(0);
		if (a == 1)
			get_com_inp(inp);
		else
		{
			int status;
			for (i = 0; i < a - 1; i++)
			{
				//printf("%s", fag[i]);
				//strcpy(inp, fag[i]);
				char flag[10][buf];
				int n = tokenise(fag[i], " ", flag);
				char *argv[100];
				int j = 0;
				for (j = 0; j < n; j++)
				{
					trim(flag[j], " ");
					argv[j] = flag[j];
				}
				argv[j] = NULL;
				int status;
				int pd[2];
				pipe(pd);
				if (!fork())
				{
					dup2(pd[1], 1);
					execvp(argv[0], argv);
					perror("exec");
					abort();
				}
				dup2(pd[0], 0);
				close(pd[1]);
			}
			char flag[10][buf];
			int n = tokenise(fag[i], " ", flag);
			char *argv[100];
			for (int i = 0; i < n; i++)
			{
				trim(flag[i], " ");
				argv[i] = flag[i];
			}
			argv[i] = NULL;
			pid_t pid = fork(), tpid;
			if (pid == 0)
			{
				execvp(argv[0], argv);
			}
			else
			{
				tpid = waitpid(pid, &status, WUNTRACED);
			}
		}

			// 		if (i == (a - 1))
			// 		{
			// 			dup2(stdoutcpy, 1);
			// 			pid_t pid = fork(), tpid;
			// 			if(pid == 0)
			// 			{
			// 				execvp(argv[0], argv);
			// 			}
			// 			else
			// 			{
			// 				tpid = waitpid(pid,&status,WUNTRACED);
			// 			}

			// 		}
			// 		else
			// 		{
			// 			int pd[2];
			// 			pipe(pd);

			// 			pid_t pid = fork();
			// 			if (pid == 0)
			// 			{
			// 				dup2(pd[1], 1);
			// 				execvp(argv[0], argv);
			// 				abort();
			// 			}
			// 			else
			// 			{
			// 				dup2(pd[0], 0);
			// 				close(pd[1]);
			// 			}

			// 			close(pd[0]);
			// 			close(pd[1]);
			// 		}
			// 		//printf("%d\n", for (i = 0; i < a; i++)
			// 	{
			// 		//printf("%s", fag[i]);
			// 		//strcpy(inp, fag[i]);
			// 		char flag[10][buf];
			// 		int n = tokenise(fag[i], " ", flag);
			// 		char *argv[100];
			// 		for (int i = 0; i < n; i++)
			// 		{
			// 			trim(flag[i], " ");
			// 			argv[i] = flag[i];
			// 		}
			// 		argv[i] = NULL;
			// 		int status;
			// 		if (i == (a - 1))
			// 		{
			// 			dup2(stdoutcpy, 1);
			// 			pid_t pid = fork(), tpid;
			// 			if(pid == 0)
			// 			{
			// 				execvp(argv[0], argv);
			// 			}
			// 			else
			// 			{
			// 				tpid = waitpid(pid,&status,WUNTRACED);
			// 			}

			// 		}
			// 		else
			// 		{
			// 			int pd[2];
			// 			pipe(pd);

			// 			pid_t pid = fork();
			// 			if (pid == 0)
			// 			{
			// 				dup2(pd[1], 1);
			// 				execvp(argv[0], argv);
			// 				abort();
			// 			}
			// 			else
			// 			{
			// 				dup2(pd[0], 0);
			// 				close(pd[1]);
			// 			}

			// 			close(pd[0]);
			// 			close(pd[1]);i);
			// 		// remap output from previous child to input
			// 	}
			// }
			dup2(stdoutcpy, 1);
			dup2(stdincpy, 0);
			CWD();
			free_flag();
		}
	}

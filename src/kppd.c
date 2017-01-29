/*
 * Copyright (c) 2015 savoca <savoca@codefi.re>
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <sys/inotify.h>
#include <limits.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h> 
#include <pthread.h>
#include "kcal.h"

#define KPPD_VER "1.1.0"
#define I_BUF_LEN (sizeof(struct inotify_event) + NAME_MAX + 1)

static int usage(char *self)
{
	printf("Usage: %s <config>\n", self);
	return 1;
}

static int version(void)
{
	printf("kppd version %s\n", KPPD_VER);
	printf("Copyright (c) 2015 savoca <savoca@codefi.re>\n");
	printf("Command input mod by craftingmod\n");
	return 0;
}

static void init_daemon(void)
{
	int i;
	pid_t pid;

	pid = fork();
	if (pid)
		exit(pid < 0 ? EXIT_FAILURE : EXIT_SUCCESS);

	umask(0);

	if (setsid() < 0 || chdir("/") < 0)
		exit(EXIT_FAILURE);

	for (i = sysconf(_SC_OPEN_MAX); i > 0; i--)
		close(i);
}

static void read_config(bool first,char *config, struct kcal_data *kcal_cfg)
{
	FILE *config_fd;
	int temp;

	config_fd = fopen(config, "r");
	if (config_fd == NULL)
		return;

	fscanf(config_fd, "[mdp_version]=%d\n", &kcal_cfg->mdp_ver);
	fscanf(config_fd, "[pa_version]=%d\n", &kcal_cfg->pa_ver);
	if(first == true){
		fscanf(config_fd, "[red]=%d\n", &kcal_cfg->rgb.red);
		fscanf(config_fd, "[green]=%d\n", &kcal_cfg->rgb.green);
		fscanf(config_fd, "[blue]=%d\n", &kcal_cfg->rgb.blue);
	} else {
		fscanf(config_fd, "[red]=%d\n", &temp);
		fscanf(config_fd, "[green]=%d\n", &temp);
		fscanf(config_fd, "[blue]=%d\n", &temp);
	}
	fscanf(config_fd, "[hue]=%d\n", &kcal_cfg->pa.hue);
	fscanf(config_fd, "[saturation]=%d\n", &kcal_cfg->pa.sat);
	fscanf(config_fd, "[value]=%d\n", &kcal_cfg->pa.val);
	fscanf(config_fd, "[contrast]=%d\n", &kcal_cfg->pa.cont);
	fscanf(config_fd, "[invert]=%d\n", &kcal_cfg->rgb.invert);

	fclose(config_fd);
}

static void apply_kcal(struct kcal_data kcal)
{
	if (kcal.mdp_ver == 5) {
		write_pcc(kcal.rgb.red, kcal.rgb.green, kcal.rgb.blue,
			kcal.rgb.invert);

		if (kcal.pa_ver == 1)
			write_pa(kcal.pa.hue, kcal.pa.sat, kcal.pa.val, kcal.pa.cont);
		else if (kcal.pa_ver == 2)
			write_pa_v2(kcal.pa.hue, kcal.pa.sat, kcal.pa.val, kcal.pa.cont);
	} else if (kcal.mdp_ver == 3)
		write_lut(kcal.rgb.red, kcal.rgb.green, kcal.rgb.blue);
}

/**
 * Sample?
 **/
static void input_rgb(char *text,struct kcal_data *kcal_cfg) {
	char *input;
	input = text;
	/* define rgbs */
	int red = 0;
	int green = 0;
	int blue = 0;
	/* split , : first */
	// define piece
	char *piece;
	// first split
	piece = strtok(input, ",");
	// null check
	if(piece != NULL){
		//println
		printf("Red: %s", piece);
		// (int)piece
		red = atoi(piece);
		kcal_cfg->rgb.red = red;
		// http://ra2kstar.tistory.com/49 : Param is null
		piece = strtok(NULL, ",");
		if(piece != NULL){
			green = atoi(piece);
			kcal_cfg->rgb.green = green;
			printf(" / Green: %s", piece);
			piece = strtok(NULL, ",");
			if(piece != NULL){
				blue = atoi(piece);
				kcal_cfg->rgb.blue = blue;
				printf(" / Blue: %s", piece);
			}
		}
		apply_kcal(*kcal_cfg);
		printf("\n");
	}
}
void *inputLoop (void *kcalObj) {
	struct kcal_data *localKcal = kcalObj;
	// length
	int ln = 13;
	// define input var (*input):one letter (input): string
	char *input;
	// define exit
	char *exitA;
	exitA = "exit";
	// notify
	printf("Command input mod by craftingmod\n");
	printf("Usage: Red,Green,Blue (211,211,211) / exit (close)\n");
	while(true){
		// length 13 (real length: 12)
		input = (char *) malloc(sizeof(char) * ln);
		// get input in console
		fgets(input, ln, stdin);
		// remove line breaker
		input[strlen(input) - 1] = '\0';
		// exit?
		if(strcmp(exitA,input) == 0){
			break;
		}
		// run inputrgb
		input_rgb(input,localKcal);
	}
	exit(0);
}

int main(int argc, char **argv)
{
	char *config;
	struct kcal_data kcal;
	struct inotify_event *event;
	int i_fd, i_wd, i_len;
	char i_buf[I_BUF_LEN];
	char i_path[PATH_MAX];
	char *p;
	// addition
	int thread_id;
	pthread_t p_thread[1];
	bool isDaemon = false;

	if (argc != 2 && argc != 3)
		return usage(argv[0]);
	if(argc == 3){
		isDaemon = true;
	}

	if (!strcmp(argv[1], "--version") || !strcmp(argv[1], "-v"))
		return version();

	config = realpath(argv[1], NULL);
	if (access(config, R_OK)) {
		printf("Could not read config %s\n", config);
		return 1;
	}

	if (access(TARGET_FB, W_OK)) {
		printf("Could not open %s\n", TARGET_FB);
		return 1;
	}
	//foreground mode
	if(isDaemon){
		init_daemon();
	}

	i_fd = inotify_init();
	if (i_fd < 0) {
		printf("Failed to initialize inotify\n");
		return 1;
	}

	i_wd = inotify_add_watch(i_fd, config,
		IN_MODIFY | IN_DELETE_SELF | IN_MOVE_SELF);
	if (i_wd < 0) {
		printf("Failed to add watch for %s\n", config);
		return 1;
	}

	read_config(true,config, &kcal);
	apply_kcal(kcal);

	//add daemon
	if(!isDaemon){
		thread_id = pthread_create(&p_thread[0], NULL, inputLoop, (void *)&kcal);
	}

	for (;;) {
		i_len = read(i_fd, i_buf, I_BUF_LEN);
		if (i_len < 1) {
			printf("Failed to read from inotify fd.\n");
			return 1;
		}

		for (p = i_buf; p < i_buf + i_len;) {
			event = (struct inotify_event *)p;

			if (event->mask & IN_MODIFY) {
				read_config(isDaemon,config, &kcal);
				apply_kcal(kcal);
			}

			if (event->mask & IN_DELETE_SELF || event->mask & IN_MOVE_SELF)
				goto exit;

			p += sizeof(struct inotify_event) + event->len;
		}
	}

exit:
	inotify_rm_watch(i_fd, i_wd);
	close(i_fd);

	return 0;
}

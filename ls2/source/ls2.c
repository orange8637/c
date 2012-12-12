/*
* Filename: ls2.c
* Last modified: 2012-12-09 23:04
* Author: Chen Jiang(姜晨) -- orange8637@gmail.com
* Description: linux homework
*/
#include<stdio.h>
#include<dirent.h>
#include<string.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<pwd.h>
#include<grp.h>
#include<getopt.h>

/*老师给定的库函数*/
char *filemode(int);
permbits(int, char *);
char *uid_to_name(short);
char *gid_to_name(short);
dostat(char *);
show_file_info(char *, struct stat *);

/*自己编写的核心功能函数*/
do_ls(char *);

/*设置是否详细显示的选项变量*/
int longlist = 0;

/*主函数*/
int main(int ac, char **av)
{
		/*捕获程序选项*/
		const char* const short_optiones = "l";
		const struct option long_options[] = { {"long list", 0, NULL, "l"} };
		int opt;
		while((opt = getopt_long(ac, av, "l", long_options, NULL)) != -1)
		{
			switch(opt)
			{
				case 'l':
					longlist = 1;
					break;
				default:
					break;
			}
		}
		int i;
		/*对程序参数处理*/
		for(i = optind; i < ac; i++)
		{
			printf("%s:\n", av[i]);
			do_ls(av[i]);
		}
		/*程序没有参数时默认显示当前目录*/
		if(ac == 1 || i == optind)
			do_ls(".");
	return 0;
}

do_ls(char *dirname)
{
	DIR *dir_ptr;
	struct dirent **direntps;
	/*搜索目录下文件项并递增排序*/
	int num = scandir(dirname, &direntps, 0, alphasort);
	if(num < 0)
		fprintf(stderr, "ls2: cannot open %s\n", dirname);
	/*不含详细选项时*/
	else if(longlist == 0)
	{
		int count = 0, i = 0;
		for(; i < num; i++)
		{
			char fname[50];
			strcpy(fname, direntps[i]->d_name);
			if(fname[0] == '.')
				continue;
			printf("%-20s", fname);
			count++;
			/*分栏显示*/
			if(count == 6)
			{
				count = 0;
				printf("\n");
			}
		}
		if(count != 0)
			printf("\n");
	}
	/*详细选项显示*/
	else
	{
		int i = 0;
		for(; i < num; i++)
		{
			char fname[50];
			strcpy(fname, direntps[i]->d_name);
			if(fname[0] == '.')
				continue;
			/*得到文件的绝对路径*/
			char fullname[100];
			strcpy(fullname, dirname);
			strcat(fullname, "/");
			strcat(fullname, fname);
			dostat(fullname);
		}
	}
}

#define LS_STYLE

char *filemode( int mode )
/*
 *	returns string of mode info
 *	default to ------- and then turn on bits
 */
{
	static	char	bits[11];
	char	type;

	strcpy( bits, "----------" );

	switch ( mode & S_IFMT ){			/* mask for type */
		case S_IFREG:	type = '-';	break;	/* stays a dash	 */
		case S_IFDIR: 	type = 'd';	break;	/* put a d there */
		case S_IFCHR:	type = 'c';	break;	/* char i/o dev	 */
		case S_IFBLK:	type = 'b';	break;	/* blk. i/o dev	 */
		default:	type = '?';	break;	/* fifo, socket..*/
	}
	bits[0] = type ;

	/* do SUID, SGID, and SVTX later */

	permbits( mode>>6 , bits+1 );			/* owner	*/
	permbits( mode>>3 , bits+4 );			/* group	*/
	permbits( mode    , bits+7 );			/* world	*/

	return bits;
}

permbits( int permval, char *string )
/*
 *	convert bits in permval into chars rw and x
 */
{
	if ( permval & 4 )
		string[0] = 'r';
	if ( permval & 2 )
		string[1] = 'w';
	if ( permval & 1 )
		string[2] = 'x';
} 

char  *uid_to_name( short uid )
/* 
 *	returns pointer to logname associated with uid, uses getpw()
 */	
{
	struct	passwd *getpwuid(), *pw_ptr;

	if ( ( pw_ptr = getpwuid( uid ) ) == NULL )
		return "Unknown" ;
	else
		return pw_ptr->pw_name ;
}

char *gid_to_name( short gid )
/*
 *	returns pointer to group number gid. used getgrgid(3)
 */
{
	struct group *getgrgid(), *grp_ptr;

	if ( ( grp_ptr = getgrgid(gid) ) == NULL )
		return "Unknown" ;
	else
		return grp_ptr->gr_name;
}

dostat( char *filename )
{
	struct stat info;

	if ( stat(filename, &info) == -1 )		/* cannot stat	 */
		perror( filename );			/* say why	 */
	else					/* else show info	 */
		show_file_info( filename, &info );
}

show_file_info( char *filename, struct stat *info_p )
/*
 * display the info about 'filename'.  The info is stored in struct at *info_p
 */
{
	//char	*uid_to_name(), *ctime(), *gid_to_name(), *filemode();
#ifdef TAGGED_STYLE

	printf("%s:\n", filename );			/* print name	 */

	printf("\t  mode: %s\n", filemode(info_p->st_mode) );
	printf("\t links: %d\n", (int) info_p->st_nlink);	/* links */
	printf("\t owner: %s\n", uid_to_name(info_p->st_uid) );
	printf("\t group: %s\n", gid_to_name(info_p->st_gid) );
	printf("\t  size: %ld\n",(long)info_p->st_size);	/* size  */
	printf("\t   mod: %s",   ctime(&info_p->st_mtime));
	printf("\taccess: %s",   ctime(&info_p->st_atime));
#endif
#ifdef	LS_STYLE
	printf( "%s"    , filemode(info_p->st_mode) );
	printf( "%4d "  , (int) info_p->st_nlink);	
	printf( "%-8s " , uid_to_name(info_p->st_uid) );
	printf( "%-8s " , gid_to_name(info_p->st_gid) );
	printf( "%8ld " , (long)info_p->st_size);
	printf( "%.12s ", 4+ctime(&info_p->st_mtime));
	printf( "%s\n"  , filename );
#endif

}

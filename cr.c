//master

#include <stdlib.h>
#include <stdio.h>
#include <conio.h>
#include <ctype.h>
#include <stdbool.h>

#define RED "\x1b[31m"
#define BLUE "\x1b[34m"
#define GREEN "\x1b[32m"
#define YELLOW "\x1b[33m"
#define MAGENTA "\x1b[35m"
#define CYAN "\x1b[36m"
#define RESET "\x1b[0m"

#ifdef _WIN32
	#define CLR_SCR system("cls");
#else
	#define CLR_SCR system("clear");
#endif

#define SLEEP(x) usleep(x*1000);

#define DISPLAY(x)\
{\
	CLR_SCR\
	show_GRID(grid);\
	SLEEP(x);\
}

#define PRINT_COLOUR(x,y,z)\
{switch(colour(y))\
	{\
		case 'r': printf(RED x RESET,y,z);\
					break;\		
		case 'b': printf(BLUE x RESET,y,z);\
					break;\		
		case 'g': printf(GREEN x RESET,y,z);\
					break;\		
		case 'y': printf(YELLOW x RESET,y,z);\
					break;\		
		case 'm': printf(MAGENTA x RESET,y,z);\
					break;\		
		case 'c': printf(CYAN x RESET,y,z);\
					break;\		
		default : printf(x,y,z);\
	}\
}

#define CREATE_SS(ptr,m,n)\
{\
	ptr=malloc(sizeof(S_S));\
	if(!ptr)\
	{printf("Memory allocation for save state failed.\n");\
		exit(1);}\
	ptr->bg=malloc(m*sizeof(B_G *));\
	if(!ptr->bg)\
	{printf("Memory allocation for backup grid ROWS failed.\n");\
		exit(1);}\
	for(i=0;i<m;i++)\
	{ptr->bg[i]=malloc(n*sizeof(B_G));\
	if(!ptr->bg[i])\
	{printf("Memory allocation for backup grid COLUMNS failed.\n");\
		exit(1);}}\
	ptr->prev=ptr->next=NULL;\
}\

int ROW=10,COL=10;

struct CELL
{
    int orbs,threshold;
    char player;
    struct CELL *top,*bottom,*left,*right;
};

struct BACKUP_GRID
{
	int orbs_b;
	char player_b;
};

struct SAVE_STATES
{
	struct BACKUP_GRID **bg;
	int current_player;
	struct SAVE_STATES *prev,*next;
	struct CELL *p_b;
};

struct PLAYERS
{
	bool exists,alive;
	char name, colour;
}player[6];

typedef struct BACKUP_GRID B_G;
typedef struct SAVE_STATES S_S;
typedef struct CELL cell;
typedef struct CELL *cellptr;

cellptr p=NULL;

int ip,SAVES,MAX_SAVES=5;
bool SHOW_EXPLOSION=1;
S_S *stateptr,*first,*last,*Add_undo;

void setup_GRID(cell[ROW][COL]);
void setup_PLAYERS();
char colour(char);
void show_GRID(cell[ROW][COL]);
int play();
void settings();
bool get_INPUT(cell[ROW][COL]);
char detect_SPLKEY(char);
void add_ORB(cell[ROW][COL],cellptr,char);
void save(cell[ROW][COL],int);
void undo(cell[ROW][COL],bool);
bool check_WINNER(cell[ROW][COL]);
char eliminate(cell[ROW][COL]);
void resize_GRID();
int cur_PLAYERS();
void change_PLAYERS();
void rules();

int main()
{
    int choice;
	CLR_SCR

	setup_PLAYERS();
	while(1)
	{
		choice=0;		

		printf("\n1.Play    2.Settings    3.Rules    4.Exit\n");
		printf("Enter your choice:");
		scanf("%d",&choice);

		switch(choice)
		{
			case 1 :while(play());
					break;

			case 2 :settings();
					break;

			case 3 :rules();
					break;

			case 4 :exit(0);

			default:printf(RED"\nInvalid choice. Please try again\n"RESET);
					while(getchar()!='\n');
		}
	}
    return 0;
}

void settings()
{
	int n;
	char choice='\0',ch;


	while(1)
	{
		printf("\n\n1.Change grid size    2.Change players    3.Change undo limit    4.Toggle cell split animation    Esc:Back to main menu\n");
		printf("Enter your choice:");
		
		while((choice=getche())!='\r' && choice!=27)
			ch=choice;

		if(choice==27)
		{
			printf("EEsc\n");
			return;
		}

		switch(ch)
		{
			case '1':resize_GRID();
					 break;

			case '2':change_PLAYERS();
					 break;
			
			case '3':printf("\n\nCurrent limit: %d",MAX_SAVES);
					 
					 while(1)
					 {
						printf("\nEnter the limit (0: disable, -ve number: unlimited. Default=5):");
						 
						if(scanf("%d",&n))
						{
							if(n==0) printf(RED"\nUndo/Redo disabled.\n"RESET);
							else if(n<0) printf(BLUE"\nUnlimited undos!"RESET);
							else printf(GREEN "Undo limit set to %d." RESET,n);
						}
						
						else
						{
							while(getchar()!='\n');
							printf(RED"Invalid input!\n"RESET);
							continue;
						}

						MAX_SAVES=n;
						break;
					 }

					 break;
			
			case '4':SHOW_EXPLOSION^=1;
					
					 if(SHOW_EXPLOSION)
					 	printf("Cell split animation " GREEN "ON" RESET".\n");
					 else
					 	printf("Cell split animation " RED "OFF" RESET".\n");
					
					 break;

			default :printf(RED"Invalid choice. Please try again\n"RESET);
		}
	}
}

void setup_GRID(cell grid[ROW][COL])
{
    int i,j,k;
    p=&grid[0][0];

    for(k=0;k<COL;k++)
    {
        for(i=0,j=ROW-1;i<ROW-1;i++,j--)
        {
            grid[i][k].bottom=&grid[i+1][k];
            grid[j][k].top=&grid[j-1][k];                                  //SET COLUMN-WISE TOP AND BOTTOM LINKS

            grid[i+1][k].orbs=0;
            grid[i+1][k].threshold=3;
            grid[i+1][k].player='0';									   //SET DEFAULT VALUES OF ORBS, THRESHOLD AND PLAYER FOR MIDDLE CELLS
        }
    }

    for(k=0;k<ROW;k++)
    {
        for(i=0,j=COL-1;i<COL-1;i++,j--)
        { 
			grid[k][i].right=&grid[k][i+1];
            grid[k][j].left=&grid[k][j-1];                                 //SET ROW-WISE LEFT AND RIGHT LINKS
        }
    }

    for(i=j=0;i<COL;i++,j++)
    {
        grid[0][i].top=NULL;                                        //SET FIRST AND LAST ROW LINKS AND THRESHHOLD
		grid[ROW-1][j].bottom=NULL;

        grid[0][i].threshold=grid[ROW-1][j].threshold=2;			//SET HORIZONTAL EDGE CELLS' DEFAULT VALUES
		grid[0][i].orbs=0;                                        //SET FIRST AND LAST ROW LINKS AND THRESHHOLD
		grid[0][j].player='0';
	}

    for(i=j=0;i<ROW;i++,j++)
    {
        grid[i][0].left=NULL;
        grid[j][COL-1].right=NULL;            //SET FIRST AND LAST COLUMN LINKS AND THRESHHOLD

		grid[i][0].threshold=grid[j][COL-1].threshold=2;				  //SET VERTICAL EDGE CELLS' DEFAULT VALUES
    }

    grid[0][0].threshold=grid[0][COL-1].threshold=grid[ROW-1][0].threshold=grid[ROW-1][COL-1].threshold=1;      //SET CORNER CELLS' THRESHOLD VALUE
}

void setup_PLAYERS()
{
	int i;
	
	player[0].exists=player[0].alive=player[1].exists=player[1].alive=1;
	player[0].name='a'; player[1].name='b';

	for(i=2;i<6;i++)
	{
		player[i].exists=player[i].alive=0;
		player[i].name='\0';
	}

	player[0].colour='r';
	player[1].colour='b';
	player[2].colour='g';
	player[3].colour='y';
	player[4].colour='m';
	player[5].colour='c';
}

char colour(char name)
{
	int i;

	for(i=0;i<6;i++)
	{
		if(name==player[i].name)
			return player[i].colour;
	}

	return '\0';
}

void show_GRID(cell grid[ROW][COL])
{
    int i,j;

    for(i=0;i<ROW;i++)
    {
        for(j=0;j<COL;j++)
        	PRINT_COLOUR("%c%d ",grid[i][j].player,grid[i][j].orbs);

        printf("\n");
    }
}

int play()
{
	bool z=1;
	cell grid[ROW][COL];
	cellptr ptr=NULL; SAVES=0;
	stateptr=first=last=Add_undo=NULL;

	for(ip=0;ip<6;ip++)
	{
		if(player[ip].exists)
			player[ip].alive=1;		

		else	player[ip].alive=0;
	}

	ip=-1;

	setup_GRID(grid);
	CLR_SCR

	do
	{
		DISPLAY(0);

		if(z)
		{
			do
			{
				ip=(ip+1)%6;
			}while(!player[ip].alive);
			
			z=!z;
			save(grid,ip);
		}
		
		if(get_INPUT(grid))
		{
			if(p->player=='&' || p->player=='$')
			{
				free(Add_undo);

				while(first!=last)
				{
					first=first->next;
					free(first->prev);
				}
				
				if (p->player=='$') return 1;

				return 0;

			}
			
			continue;
		}
				
		ptr=p;

		if(ptr->player==player[ip].name || ptr->player=='0')
		{
			while(last!=stateptr)
			{
				last=last->prev;
				free(last->next);
			}
			
			add_ORB(grid,ptr,player[ip].name);
			z=1;	
		}
		
		else
		{
			printf(RED"Cell already occupied by player %c. Try a different cell.\n"RESET,player[ip].name);
			SLEEP(1000);
		}
	}while(!check_WINNER(grid));

	free(Add_undo);

	while(first!=last)
	{
		first=first->next;
		free(first->prev);
	}

	return 0;
}

bool get_INPUT(cell grid[ROW][COL])
{
    int temp1; char dir='0',temp2;

    do
    {
    	temp1=p->orbs;
		temp2=p->player;

		p->orbs=1;
		p->player='+';

		DISPLAY(0);

		printf("<player %c>:",player[ip].name);

		printf("\n\nWASD/arrow keys to move, ENTER to select cell. \nz to undo, r to redo. 'Esc' to exit to main menu.\n");
		dir=getch();
        dir=detect_SPLKEY(dir);

        switch(dir)
        {
            case 'w':if(p->top)
                     {
                         p->orbs=temp1;
                         p->player=temp2;
                         p=p->top;
                     }

                     else
					 {
					 	 p->orbs=temp1;
						 p->player=temp2;
					 }

                     break;

            case 'a':if(p->left)
                     {
                         p->orbs=temp1;
                         p->player=temp2;
                         p=p->left;
                     }

                     else
					 {
					 	 p->orbs=temp1;
						 p->player=temp2;
					 }

                     break;

            case 's':if(p->bottom)
                     {
                         p->orbs=temp1;
                         p->player=temp2;
                         p=p->bottom;
                     }

                     else
					 {
					 	 p->orbs=temp1;
						 p->player=temp2;
					 }

                     break;


            case 'd':if(p->right)
                     {
                         p->orbs=temp1;
                         p->player=temp2;
                         p=p->right;
                     }

                     else
					 {
					 	 p->orbs=temp1;
						 p->player=temp2;
					 }

                     break;

			case 'z':p->orbs=temp1;
					 p->player=temp2;
					 
					 undo(grid,1);

					 return 1;

			case 'r':p->orbs=temp1;
					 p->player=temp2;
					 					 
					 undo(grid,0);
					 
					 return 1;

			case '#':printf("Press 'Esc' to return to main menu. Press 'r' to restart. Press any other key to cancel.\n");
					 dir=getch();
					 dir=detect_SPLKEY(dir);

					 if(dir=='#')
					 {
						 p->player='&';
						 return 1;
					 }

					 else if(dir=='r')
					 {
						 p->player='$';
						 return 1;
					 }

					 else
					 {
					 	 p->orbs=temp1;
						 p->player=temp2;

						 dir='\0';
					 }

					 continue;

			case '\n':
			case '\r':break;

			case 't':setup_GRID(grid);

					 grid[4][4].orbs=grid[5][4].orbs=grid[6][4].orbs=grid[5][3].orbs=3;
					 grid[4][4].player=grid[5][4].player=grid[6][4].player=grid[5][3].player='a';

					 grid[4][5].orbs=grid[5][5].orbs=grid[6][5].orbs=grid[5][6].orbs=3;
					 grid[4][5].player=grid[5][5].player=grid[6][5].player=grid[5][6].player='b';

					 p=&grid[5][4];
					 ip=0;

					 DISPLAY(500);

					 break;

            default :printf(RED"Invalid choice. Try again.\n"RESET);
					 SLEEP(1000);

					 p->orbs=temp1;
					 p->player=temp2;

                     break;
        }
    }while(dir!='\n' && dir!='\r');

	p->orbs=temp1;
    p->player=temp2;

	return 0;
}

char detect_SPLKEY(char c)
{
	if(c==-32 || c==0)
	{
		switch(getch())
		{
			case 72: return 'w';
			case 77: return 'd';
			case 80: return 's';
			case 75: return 'a';
		}
	}

	else if(c==27) return '#';
		 else return c;
}

void add_ORB(cell grid[ROW][COL],cellptr ptr,char player)
{
	int i,j;
	bool unstable=0;
	cellptr a=NULL; S_S *saveptr=NULL;

	ptr->orbs++;
	ptr->player=player;

	DISPLAY(500);

	if(ptr->orbs>ptr->threshold)
	{
		unstable=1;
		save(grid,-1);
	}

	saveptr=Add_undo;

	while(unstable)
	{
		unstable=0;		
		ptr=grid;

		for(i=0;i<ROW;i++)
		{
			for(j=0;j<COL;j++)
			{
				if(saveptr->bg[i][j].orbs_b>ptr->threshold)
				{
					ptr->orbs-=(ptr->threshold+1);
					if(!ptr->orbs)
						ptr->player='0';

					unstable=1;
					saveptr->bg[i][j].player_b='#';

					ptr++;
					continue;
				}

				if(saveptr->bg[i][j].player_b=='#')
				{
					saveptr->bg[i][j].player_b='\0';

					if(ptr->top)
					{
						ptr->top->player=player;
						ptr->top->orbs++;
						if(ptr->top->orbs>ptr->top->threshold)
							unstable=1;
					}

					if(ptr->bottom)
					{
						ptr->bottom->player=player;
						ptr->bottom->orbs++;
						if(ptr->bottom->orbs>ptr->bottom->threshold)
							unstable=1;
					}

					if(ptr->left)
					{
						ptr->left->player=player;
						ptr->left->orbs++;
						if(ptr->left->orbs>ptr->left->threshold)
							unstable=1;
					}

					if(ptr->right)
					{
						ptr->right->player=player;
						ptr->right->orbs++;
						if(ptr->right->orbs>ptr->right->threshold)
							unstable=1;
					}
				}

				ptr++;
			}

		}

		save(grid,-1);

		if(SHOW_EXPLOSION)
			DISPLAY(500);

	}

	DISPLAY(0);
}

void save(cell grid[ROW][COL],int ip)
{
	int i,j;
	S_S *temp;
	
	if(ip==-1)
	{
		if(!Add_undo) CREATE_SS(Add_undo,ROW,COL);
		temp=Add_undo;
	}
	
	else
	{
		CREATE_SS(temp,ROW,COL);
		SAVES++;

		if(!stateptr) 
		{
			stateptr=first=last=temp;
			temp->prev=temp->next=NULL;
		}

		else
		{
			stateptr->next=temp;
			temp->prev=stateptr;
			temp->next=NULL;
			stateptr=last=temp;

			if(SAVES>MAX_SAVES+1 && MAX_SAVES>=0)
			{
				SAVES--;
				first=first->next;
				free(first->prev);
				first->prev=NULL;
			}
		}
	
		temp->current_player=ip;
		temp->p_b=p;
	}

	for(i=0;i<ROW;i++)
    {
        for(j=0;j<COL;j++)
        {
			temp->bg[i][j].orbs_b=grid[i][j].orbs;
			
			if(ip==-1)
			{
				if(temp->bg[i][j].player_b!='#') temp->bg[i][j].player_b='0';
			}
			
			else temp->bg[i][j].player_b=grid[i][j].player;
        }
    }
}

void undo(cell grid[ROW][COL],bool toggle)
{
	int i,j; char temp_player; S_S *temp;

	if(toggle)
	{
		if(!stateptr || !stateptr->prev)
		{
			if(MAX_SAVES==0)
				printf(RED"Undos disabled!\n"RESET);

			else
				printf(RED"Undo limit reached!\n"RESET);
			SLEEP(600);
			return;
		}
		temp=stateptr->prev;
	}

	else
	{
		if(!stateptr || !stateptr->next )
		{
			if(MAX_SAVES==0)
				printf(RED"Redos disabled!\n"RESET);

			else
				printf(RED"Redo limit reached!\n"RESET);
			SLEEP(600);
			return;
		}

		temp=stateptr->next;
	}
	

	for(i=0;i<ROW;i++)
    {
        for(j=0;j<COL;j++)
        {
				grid[i][j].orbs=temp->bg[i][j].orbs_b;
				grid[i][j].player=temp->bg[i][j].player_b;
        }
    }

	if(toggle)
	{
		i=stateptr->p_b->orbs;
		temp_player=stateptr->p_b->player;
		
		stateptr->p_b->orbs=1;
		stateptr->p_b->player='-';

		DISPLAY(400);

		stateptr->p_b->orbs=i;
		stateptr->p_b->player=temp_player;

		SAVES--;
	}

	else
	{
		i=temp->p_b->orbs;
		temp_player=temp->p_b->player;
		
		temp->p_b->orbs=1;
		temp->p_b->player='+';

		DISPLAY(300);

		temp->p_b->orbs=i;
		temp->p_b->player=temp_player;

		SAVES++;
	}
	

	DISPLAY(400);

	ip=temp->current_player;
		
	stateptr=temp;
}

bool check_WINNER(cell grid[ROW][COL])
{
	char winner='\0';

	winner=eliminate(grid);

    if(winner)
	{
		printf("Player %c wins!\n",winner);
		return 1;
	}

	return 0;
}

char eliminate(cell grid[ROW][COL])
{
	cellptr ptr[6]={NULL};
	bool alive[6]={0},same=0;
	int ip,i=0; char winner='\0';

	for(ip=0;ip<6;ip++)
	{
		if(player[ip].alive)
		{
			ptr[ip]=grid;

			while(ptr[ip]<=&grid[ROW-1][COL-1])
			{
				if(ptr[ip]->player==player[ip].name)
				{
					if(alive[ip]==1)
					{
						same=1;
						break;
					}
					
					alive[ip]=1;
				}

				ptr[ip]++;
			}
		}
	}

	if(same)
	{
		for(ip=0;ip<6;ip++)
		{
			if(!alive[ip])
				player[ip].alive=0;

			else 
			{
				i++;
				if(!winner) winner=player[ip].name;
			}
		}
	}

	if(i==1) return winner;
	else return '\0';
}

void resize_GRID()
{
	int m,n;

	while(1)
	{
		printf("\n\nEnter the row and column size:");
		scanf("%d%d",&m,&n);

		if(m>=5 && m<=20)
		{
			if(n>=5 && n<=20)
			{
				ROW=m; COL=n;
				printf(GREEN"Size changed successfully!\n"RESET);

				return;
			}
		}

		while(getchar()!='\n');
		printf(RED"Both values must be between 5 and 20. Please try again.\n"RESET);
	}
}

int cur_PLAYERS()
{
	int i,count=0;;
	printf("\nCurrent players: ");
	for(i=0;i<6;i++)
	{
		if(player[i].exists)
		{
			count++;
			PRINT_COLOUR("%c%c ",player[i].name,' ');
		}
	}

	return count;
}

void change_PLAYERS()
{
	int ip,count=0;
	char name,colour;
	
	CLR_SCR
	count=cur_PLAYERS();
	
	while(1)
	{
		printf("\n\nPress '+' to add a player, and '-' to remove player.\n");
		printf("To modify details of an existing player, enter their name (Press Esc key to exit):");
		name=getche();
		
		if(name==27)
		{
			printf("1");
			break;
		}
		
		else if(name=='+')
		{
			while(count<6)
			{
				while(1)
				{
					printf("\n\nEnter a single character name for the player (Press Esc key to go back):");
					name=getche();

					if(name==27)
					{
						CLR_SCR
						cur_PLAYERS();

						ip=-1;
						break;
					}

					if(isalpha(name))
					{
						for(ip=0;ip<6;ip++)
						{
							if(player[ip].name==name)
							{
								printf(RED"\nName already taken by another user. Please try another name.\n"RESET);
								break;
							}
						}

						if (ip==6) break;
					}
					
					else printf(RED"\nInvalid name! Please enter an uppercase or lowercase alphabet only.\n"RESET);
				}
				
				if(ip==-1)	break;

				while(1)
				{
					printf("\n\nEnter the colour {");
					for(ip=0;ip<6;ip++)
					{
						if(!player[ip].exists)
						{
							printf("'%c':",player[ip].colour);
							
							switch (player[ip].colour)
							{
								case 'r': printf(RED "red" RESET ", ");
											break;
								case 'b': printf(BLUE "blue" RESET ", ");
											break;	
								case 'g': printf(GREEN "green" RESET ", ");
											break;
								case 'y': printf(YELLOW "yellow" RESET ", ");
											break;	
								case 'm': printf(MAGENTA "magenta" RESET ", ");
											break;	
								case 'c': printf(CYAN "cyan" RESET ", ");
											break;
							}
						}
					}
					printf("] (Press Esc key to go back):");
					
					colour=getche();				
					
					if(colour==27)
					{
						CLR_SCR
						cur_PLAYERS();
						
						ip=-1;
						break;
					}

					for(ip=0;ip<6;ip++)
					{
						if(player[ip].colour==colour)
						{
							if(!player[ip].exists)
							{
								player[ip].exists=player[ip].alive=1;
								player[ip].name=name;
								
								printf(GREEN"\nPlayer added!\n"RESET);
								cur_PLAYERS();
								
								ip=-2;
								count++;
							}

							else
							{
								printf(RED"\nPlayer with that colour already exists. Please try another colour.\n"RESET);
								ip=-3;
							}

							break;			
						}
					}
						
					if(ip==-2)	break;
					else if(ip==-3) continue;

					printf(RED"\nInvalid colour code. Please enter a single character according to the colour code shown below:\n"RESET);
				}

				if(ip==-1)	break;
			}

			if(ip==-1) continue;
			
			printf(RED"\nMaximum number of players reached!\n"RESET);
		}

		else if(name=='-')
		{
			while(count>2)
			{
				printf("\n\nEnter the character name of the player to be removed (Press Esc key to exit):");
				name=getche();

				if(name==27)
				{
					CLR_SCR
					cur_PLAYERS();

					ip=-1;
					break;
				}
				
				for(ip=0;ip<6;ip++)
				{
					if(player[ip].name==name)
					{
						printf("\nPress 'Y' to confirm:");
						scanf("%c",&colour);

						if(colour=='Y' || colour =='y')
						{					
							player[ip].name='\0';
							player[ip].exists=player[ip].alive=0;

							printf(GREEN"\nPlayer removed!\n"RESET);
							cur_PLAYERS();

							count--;
						}

						ip=-2;
						break;
					}
				}

				if(!isalpha(name))
				{
					printf(RED"\nInvalid name! Please enter an uppercase or lowercase alphabet only.\n"RESET);
					continue;
				}

				if(ip==-2) continue;

				printf(RED"\nPlease enter a valid name of an existing player.\n"RESET);
			}

			if(ip==-1)	continue;

			printf(RED"\nMinimum number of players reached!\n"RESET);
		}

		else
		{
			int i;
			ip=0;
			
			for(i=0;i<6;i++)
			{
				if(player[i].name==name)
				{					
					while(1)
					{
						printf("\n\nEnter the new name (Press Esc key to go back):");
						name=getche();

						if(name==27)
						{
							printf("P");
							CLR_SCR
							cur_PLAYERS();
						
							ip=-1;
							break;
						}

						if(name==detect_SPLKEY(name) && isalpha(name))
						{
							for(ip=0;ip<6;ip++)
							{
								if(player[ip].name==name && ip!=i)
								{
									printf(RED"\nExisting name. Please try another name.\n"RESET);
									break;
								}
							}

							if (ip==6) break;
						}
						
						else printf(RED"\nInvalid name! Please enter an uppercase or lowercase alphabet only.\n"RESET);
					}

					if(ip==-1) break;
					
					while(1)
					{
						player[i].exists=0;
						
						printf("\n\nEnter the new colour {");
						for(ip=0;ip<6;ip++)
						{
							if(!player[ip].exists)
							{
								printf("'%c':",player[ip].colour);
								
								switch (player[ip].colour)
								{
									case 'r': printf(RED "red" RESET ", ");
												break;
									case 'b': printf(BLUE "blue" RESET ", ");
												break;	
									case 'g': printf(GREEN "green" RESET ", ");
												break;
									case 'y': printf(YELLOW "yellow" RESET ", ");
												break;	
									case 'm': printf(MAGENTA "magenta" RESET ", ");
												break;	
									case 'c': printf(CYAN "cyan" RESET ", ");
												break;
								}
							}
						}
						
						player[i].exists=1;

						printf("] (Press Esc key to go back):");

						colour=getche();

						if(colour==27)
						{
							printf("P");
							CLR_SCR
							cur_PLAYERS();
						
							ip=-1;
							break;
						}

						for(ip=0;ip<6;ip++)
						{
							if(player[ip].colour==colour)
							{
								if(!player[ip].exists || ip==i)
								{
									player[i].exists=player[i].alive=0;
									player[i].name='\0';

									player[ip].exists=player[ip].alive=1;
									player[ip].name=name;
									
									printf(GREEN"\nPlayer modified!\n"RESET);
									cur_PLAYERS();
									
									ip=-1;
								}

								else
								{
									printf(RED"\nPlayer with that colour already exists. Please try another colour.\n"RESET);
									ip=-2;
								}

								break;			
							}
						}
							
						if(ip==-1)	break;
						else if(ip==-2) continue;

						printf(RED"\nInvalid colour code. Please enter a single character according to the colour code shown below:\n"RESET);
					}

					if(ip==-1) break;
						
				}
			}
			
			if(ip==-1) continue;

			printf(RED"\nPlease enter a valid name of an existing player.\n"RESET);
		}
	}

	CLR_SCR
}

void rules()
{
	CLR_SCR
	printf("\nChain Reaction is turn-based game for 2-6 people. It consists of a two-dimensional grid and many 'spheres' or 'orbs'.\n\n");
	printf("Each cell in the grid is represented as a combination of two items:\n");
	printf("->The player whose orbs are in that cell ('0' if cell is unoccupied)\n->The number of orbs in that cell (again, defaults to 0)\n");
	printf("The line below the grid specifies whose turn is it to play the game.\n\n");
	printf("In each player's turn, the respective player is given one orb to place in one of the cells of the grid. ");
	printf("The orb can only be placed in a cell where there are no orbs belonging to the opponent.\n");
	printf("Move the cursor using arrow keys or \"wasd\". To select a cell, press 'Enter'.\n");
	printf("\nWhen a particular cell is selected, one orb is added to that cell. ");
	printf("If the cell orb count exceeds that of the threshold value for that cell, then that cell becomes unstable and explodes.\n");
	printf("Threshold value for:-\n->Corner cells: 1\n->Edge cells: 2\n->Other cells: 3\n\n");
	printf("When a cell explodes, its orbs split and travel in as much of the four directions as possible. ");
	printf("One orb travels to each surrounding cell that exists, any excess orbs stay in the cell.\n");
	printf("Thus, orb count of the neighboring cells increase, and along with that, the player they belong to also switches to the player who caused the explosion.\n");
	printf("If the neighboring cells become unstable after the explosion, they explode too, resulting in a chain reaction.\n\n");
	printf("The game ends when the all but one player is completely eliminated from the grid, and the remaining player is declared as winner.");
	getch();

	CLR_SCR

	return;
}

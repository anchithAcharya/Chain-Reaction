#include <stdlib.h>
#include <stdio.h>
#include <conio.h>
#include <windows.h>
#include <stdbool.h>

#define RED "\x1b[31m"
#define BLUE "\x1b[34m"
#define RESET "\x1b[0m"

#define PAUSE Sleep(500);
#define PAUSE2 getch();

#define CREATE_SS(ptr,m,n)\
{\
	ptr=malloc(sizeof(S_S));\
	ptr->bg=malloc(m*sizeof(B_G *));\
	for(i=0;i<m;i++)\
		ptr->bg[i]=malloc(n*sizeof(B_G));\
	ptr->prev=ptr->next=NULL;\
}

int ROW=10,COL=10;

struct CELL
{
    int orbs,threshold,u1,v1;
    char player,u2,v2;
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
	char current_player;
	struct SAVE_STATES *prev,*next;
};

typedef struct BACKUP_GRID B_G;
typedef struct SAVE_STATES S_S;
typedef struct CELL cell;
typedef struct CELL *cellptr;

cellptr p=NULL;

int SAVES=0;
S_S *stateptr,*first,*Add_undo=NULL;

void setup_GRID(cell[ROW][COL]);
void show_GRID(cell[ROW][COL]);
void play();
bool get_INPUT(cell[ROW][COL],bool*);
char detect_SPLKEY(char);
void add_ORB(cell[ROW][COL],cellptr,char);
void save(cell[ROW][COL],char);
void undo(cell[ROW][COL],bool*);
char check_WINNER(cell[ROW][COL]);
void resize_GRID();
void rules();

int main()
{
    int choice=0;


	while(1)
	{
		printf("\n1.Play    2.Change grid size    3.Rules    4.Exit\n");
		printf("Enter your choice:");
		scanf("%d",&choice);

		switch(choice)
		{
			case 1 :play();
					break;

			case 2 :resize_GRID();
					break;

			case 3 :rules();
					break;

			case 4 :exit(0);

			default:printf("Invalid choice. Please try again\n");
		}
	}
    return 0;
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

            grid[i+1][k].u1=0;
            grid[i+1][k].u2='0';
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
		grid[0][i].u1=0;grid[0][i].u2='0';
	}

    for(i=j=0;i<ROW;i++,j++)
    {
        grid[i][0].left=NULL;
        grid[j][COL-1].right=NULL;            //SET FIRST AND LAST COLUMN LINKS AND THRESHHOLD

		grid[i][0].threshold=grid[j][COL-1].threshold=2;				  //SET VERTICAL EDGE CELLS' DEFAULT VALUES
    }

    grid[0][0].threshold=grid[0][COL-1].threshold=grid[ROW-1][0].threshold=grid[ROW-1][COL-1].threshold=1;      //SET CORNER CELLS' THRESHOLD VALUE
	}

void show_GRID(cell grid[ROW][COL])
{
    int i,j;

    for(i=0;i<ROW;i++)
    {
        for(j=0;j<COL;j++)
        {
            if(grid[i][j].player=='a')
				printf(RED"%c%d "RESET,grid[i][j].player,grid[i][j].orbs);
			else if(grid[i][j].player=='b')
				printf(BLUE"%c%d "RESET,grid[i][j].player,grid[i][j].orbs);
			else
				printf("%c%d ",grid[i][j].player,grid[i][j].orbs);
        }

        printf("\n");
    }
}

void play()
{
	int i;
	bool a=1;
	cell grid[ROW][COL]; cellptr ptr=NULL;
	stateptr=first=NULL;

	setup_GRID(grid);

	while(!check_WINNER(grid))
	{
		system("cls");
		show_GRID(grid);
		
		if(get_INPUT(grid,&a))
		{
			if(p->player=='&') return;

			continue;
		}
				
		ptr=p;

		if(a)
		{
			if(ptr->player!='b')
			{
				save(grid,'a');
				add_ORB(grid,ptr,'a');
			}

			else
			{
				printf(RED"Cell already occupied by player b. Try a different cell.\n"RESET);
				Sleep(1000);
				continue;
			}

		}

		else
		{
			if(ptr->player!='a')
			{
				save(grid,'b');
				add_ORB(grid,ptr,'b');
			}

			else
			{
				printf(RED"Cell already occupied by player a. Try a different cell.\n"RESET);
				Sleep(1000);
				continue;
			}

		}

		a=!a;
	}

	free(Add_undo);
}

bool get_INPUT(cell grid[ROW][COL],bool *a)
{
    int temp1; char dir='0',temp2;

    do
    {
    	temp1=p->orbs;
		temp2=p->player;

		p->orbs=1;
		p->player='+';

		system("cls");
		show_GRID(grid);

		if(*a)
			printf("<player a>:");
		else
			printf("<player b>:");

		printf("\n\n(WASD to move, ENTER to select cell. Z to undo.)\n");
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
					 
					 undo(grid,a);
					 
					 return 1;

			case '#':printf("Exit game? Press 'Esc' once again to confirm. Press any other key to cancel.\n");
					 dir=getch();
					 dir=detect_SPLKEY(dir);

					 if(dir=='#')
					 {
						 p->player='&';
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

					 system("cls");
					 show_GRID(grid);
					 PAUSE

					 break;

            default :printf(RED"Invalid choice. Try again.\n"RESET);
					 Sleep(1000);

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

	system("cls");
	show_GRID(grid);
	PAUSE

	if(ptr->orbs>ptr->threshold)
	{
		unstable=1;
		save(grid,'v');
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

		save(grid,'v');

		system("cls");
		show_GRID(grid);
		PAUSE

	}
}

void save(cell grid[ROW][COL],char t)
{
	int i,j;
	S_S *temp;
	
	if(t=='v')
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
			stateptr=first=temp;
			temp->prev=temp->next=temp;
		}

		else
		{
			stateptr->next=temp;
			temp->prev=stateptr;
			temp->next=first;
			first->prev=temp;
			stateptr=temp;

			if(SAVES>5)
			{
				SAVES--;
				first=first->next;
				free(first->prev);
				first->prev=temp;
			}
		}
	
		temp->current_player=t;
	}

	for(i=0;i<ROW;i++)
    {
        for(j=0;j<COL;j++)
        {
			temp->bg[i][j].orbs_b=grid[i][j].orbs;
			
			if(t=='v')
			{
				if(temp->bg[i][j].player_b!='#') temp->bg[i][j].player_b='0';
			}
			
			else temp->bg[i][j].player_b=grid[i][j].player;
        }
    }
}

void undo(cell grid[ROW][COL],bool *a)
{
	int i,j;
	if(!stateptr)
	{
		printf("Undo limit reached!\n");
		Sleep(400);
		return;
	} 

	for(i=0;i<ROW;i++)
    {
        for(j=0;j<COL;j++)
        {
				grid[i][j].orbs=stateptr->bg[i][j].orbs_b;
				grid[i][j].player=stateptr->bg[i][j].player_b;
        }
    }

	system("cls");
    show_GRID(grid);
    Sleep(800);

	if(stateptr->current_player=='a')					//TODO: remove this when implementing multiple players
		*a=1;
	else
		*a=0;
		
	if(first==stateptr)
	{
		free(stateptr);
		first=stateptr=NULL;
	}	

	else
	{
		stateptr=stateptr->prev;
		free(stateptr->next);
		stateptr->next=first;
		first->prev=stateptr;
	}

	SAVES--;
}

char check_WINNER(cell grid[ROW][COL])
{
	int i,j; char temp='\0'; bool same=0;

	for(i=0;i<ROW;i++)
    {
        for(j=0;j<COL;j++)
        {
        	if(!temp)
			{
				if(grid[i][j].player!='0')
					temp=grid[i][j].player;
			}

			else
			{
				if(grid[i][j].player!=temp)
				{
					if(grid[i][j].player!='0')
						return '\0';
				}

				else
					same=1;
			}
        }
    }

    if(temp)
	{
		if(same)
		{
			printf("Player %c wins!\n",temp);
			return temp;
		}

	}

	return '\0';
}

void resize_GRID()
{
	int m,n;

	while(1)
	{
		printf("\nEnter the row and column size:");
		scanf("%d%d",&m,&n);

		if(m>=5 && m<=20)
		{
			if(n>=5 && n<=20)
			{
				ROW=m; COL=n;
				printf("Size changed successfully!\n");

				return;
			}
		}

		printf(RED"Both values must be between 5 and 20. Please try again.\n"RESET);
	}
}

void rules()
{
	system("cls");
	printf("\nChain Reaction is turn-based game for two people. It consists mainly of a two-dimensional grid and many 'spheres' or 'orbs'.\n\n");
	printf("Each cell in the grid is represented as a combination of two items:\n");
	printf("->The player whose orbs are in that cell ('0' if cell is unoccupied)\n->The number of orbs in that cell (again, defaults to 0)\n");
	printf("The line below the grid specifies whose turn is it to play the game.\n\n");
	printf("In each player's turn, the respective player is given one orb to place in one of the cells of the grid. ");
	printf("The orb can only be placed in a cell where there are no orbs belonging to the opponent.\n");
	printf("As such, the grid switches to 'selection mode', where the player can move through the grid by the means of a pointer represented as '+1'.\n");
	printf("The player can press 'w', 'a', 's', and 'd' to move up, left, down and right respectively through the cells. To select a cell, press 'Enter'.\n");
	printf("\nWhen a particular cell is selected, an orb is added to that cell. ");
	printf("If the cell orb count exceeds that of the threshold value for that cell, then that cell becomes unstable and explodes.\n");
	printf("Threshold value for:-\n->Corner cells: 1\n->Edge cells: 2\n->Other cells: 3\n\n");
	printf("When a cell explodes, its orbs split and travel in as much of the four directions as possible. ");
	printf("One orb travels to each surrounding cell that exists, any excess orbs stay in the cell.\n");
	printf("Thus, orb count of the neighboring cells increase, and along with that, the player they belong to also switches to the player who caused the explosion.\n");
	printf("If the neighboring cells become unstable after the explosion, they explode too, resulting in a chain reaction.\n\n");
	printf("The game ends when the a player is completely eliminated from the grid, and the other player is declared as winner.");
	getch();

	system("cls");

	return;
}

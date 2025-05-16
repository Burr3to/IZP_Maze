#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

enum PathType
{
    LPATH = 2,
    RPATH = 1,
    SHORTEST = 3
};

typedef enum
{
    LEFT = 4,
    RIGHT = 6,
    FORWARD = 8,
    DOWN = 2
} Direction;

typedef struct
{
    int rows;
    int cols;
    unsigned char *cells;
} Map;

bool checkInput(char *arguments, int Xstart, int Ystart, enum PathType *path);
void getHelp();
void toBinary(int number, int *array);
void fillMapCells(FILE *bludiste, Map *map);
void goRight(Direction *facing, int Xcurr, int Ycurr);
void turnLeft(Direction *facing, int Xcurr, int Ycurr);
void goLeft(Direction *facing, int Xcurr, int Ycurr);
void turnRight(Direction *facing, int Xcurr, int Ycurr);
void RightHand(Map *map, int startborder, int Xcurr, int Ycurr);
void LeftHand(Map *map, int startborder, int Xcurr, int Ycurr);

int isMazeValid(FILE *bludiste, Map *maze, int R, int C);
int moveForward(int *Xcurr, int *Ycurr, Direction facing, Map *map);
int mapPossibleExits(Map *map, int r, int c, int *exitsX, int *exitsY, int Xstart, int Ystart);

bool isborder(Map *map, int r, int c, int border);
int start_border(Map *map, int r, int c, int leftright);

// print help with program
void getHelp()
{
    const char *help = "syntax: ./maze [argument] [R C] [filename.txt] \n"
                       "[argument] (only one of)  -- help || --test || --lpath || --rpath || --shortest \n"
                       "R C        > R is starting row, C is starting column, both > 0 \n"
                       "--help     > prints syntax of this this program \n"
                       "--test     > test if maze if valid, File is needed with this selection \n"
                       "--rpath    > path through the maze going only right, starting at row R and column C printing cells it passed \n"
                       "           > R, C, File is needed with this selection \n"
                       "--lpath    > path through the maze going only left, starting at row R and column C printing cells it passed \n"
                       "           > R, C, File is needed with this selection \n"
                       "--shortest > shortest path through the maze\n"
                       "           > R, C, File is needed with this selection \n";

    printf("%s", help);
}

// check Rows Cols equal ones in maze, checking borders, checking values in maze
int isMazeValid(FILE *bludiste, Map *maze, int R, int C)
{
    maze->cells = malloc((R * C) * sizeof(unsigned char));
    if (maze->cells == NULL)
    {
        fprintf(stdout, "Memory allocation failed");
        return 0;
    }

    int buffer, RowCheck = 0, ColCheck = 0, TotalCheck = 0;
    while ((buffer = fgetc(bludiste)) != EOF && TotalCheck < (R * C)) // go through remaining values in file
    {
        if (isspace(buffer)) // ignoring spaces
            continue;
        if (buffer < '0' || buffer > '7') // unsupported numbers
            return 0;

        // update for cols then reset and increment rows
        ColCheck++;
        if (ColCheck == C) // counting Col
        {
            ColCheck = 0;
            RowCheck++;
        }

        // filing maze step by step
        maze->cells[TotalCheck++] = buffer - '0';
        // convert a character representing a digit ('0' to '7') to int value
    }

    if (RowCheck != R || TotalCheck != R * C) // rows not equal to set R
        return 0;

    maze->rows = R;
    maze->cols = C;

    // checking borders with adjecent cells and up cells
    for (int i = (maze->rows - 1); i >= 0; i--)
    {
        for (int j = 0; j <= (maze->cols - 1); j++)
        {
            if (j < 6 && isborder(maze, i, j + 1, LEFT) != isborder(maze, i, j, RIGHT)) // rigth with left
                return 0;
            if (i > 1 && isborder(maze, i - 1, j, DOWN) != isborder(maze, i, j, FORWARD) && (i + j) % 2 == 0) // vetical check
                return 0;
        }
    }

    return 1;
}

// checks for valid starting positions, sets PathType for later use
bool checkInput(char *arguments, int Xstart, int Ystart, enum PathType *path)
{
    if (Xstart > 0 && Ystart > 0)
    {
        if (strcmp(arguments, "--lpath") == 0)
        {
            *path = LPATH;
            return true;
        }
        else if (strcmp(arguments, "--rpath") == 0)
        {
            *path = RPATH;
            return true;
        }
        else if (strcmp(arguments, "--shortest") == 0)
        {
            *path = SHORTEST;
            return true;
        }
    }
    return false;
}

// to Binary
void toBinary(int number, int *array)
{
    // check for not supported numbers
    if (number < 0 || number > 7)
    {
        fprintf(stderr, "Invalid input: %d (expected range 0-7)\n", number);
        return;
    }
    int i = 0;

    // handling zero case, just for myself
    if (number == 0)
    {
        array[0] = 0;
        i = 1;
    }
    // Convert decimal to binary
    while (number > 0)
    {
        array[i++] = number % 2;
        number /= 2;
    }

    while (i < 3) // fill remaining bits with 0
        array[i++] = 0;

    // Reverse
    for (int start = 0, end = i - 1; start < end; start++, end--)
    {
        int temp = array[start];
        array[start] = array[end];
        array[end] = temp;
    }
}

// check border for specified direction
bool isborder(Map *map, int r, int c, int border)
{
    // accesing indexes out of bounds
    if (r < 0 || r >= map->rows || c < 0 || c >= map->cols)
    {
        printf("%d < 0, %d >= %d,  %d < 0, %d >= %d\n", r, r, map->rows, c, c, map->cols);
        fprintf(stderr, "Trying to access element outside of bounds\n");
        return 0;
    }

    int array[3] = {0};
    int number = map->cells[r * map->cols + c]; // formula for getting element in 2D array stored in 1D
    toBinary(number, array);

    // Check if the specified border is RIGHT or LEFT
    if (border == RIGHT)
        return array[1];
    else if (border == LEFT)
        return array[2];

    // Check if the specified border is FORWARD or DOWn
    if ((r + c) % 2 == 0) // Even
        return (array[0] && (border == FORWARD));
    else
        return (array[0] && (border == DOWN));

    return false;
}

// starting direction
int start_border(Map *map, int r, int c, int leftright)
{
    switch (leftright)
    {
    case RPATH:
        if (r % 2 != 0 && c == 1) // 1 priorita etc
            return RIGHT;
        else if (r % 2 == 0 && c == 1) // 2
            return DOWN;
        else if (c == map->cols && (r + c) % 2 == 0) // 5
            return FORWARD;
        else if (c == map->cols && (r + c) % 2 != 0) // 6
            return LEFT;
        else if (r == 1 && c % 2 != 0) // 3
            return LEFT;
        else if (r == map->rows && c % 2 != 0) // 4
            return RIGHT;
        break;
    case LPATH:
        if (r % 2 != 0 && c == 1 && !isborder(map, r, c, FORWARD)) // 1
            return FORWARD;
        else if (r % 2 == 0 && c == 1) // 2
            return RIGHT;
        else if (c == map->cols && (r + c) % 2 == 0) // 5
            return LEFT;
        else if (c == map->cols && (r + c) % 2 != 0) // 6
            return DOWN;
        else if (r == 1) // 3
            return RIGHT;
        else if (r == map->rows && (r + c) % 2 != 0) // 4
            return LEFT;
        break;
    }
    return 0;
}

// move forwards depending on the direction in maze
int moveForward(int *Xcurr, int *Ycurr, Direction facing, Map *map)
{
    if ((*Xcurr + *Ycurr) % 2 == 0) // normal triangle
    {
        if (facing == FORWARD)
        {
            (*Xcurr)--;
        }
        else if (facing == RIGHT)
        {
            (*Ycurr)++;
        }
        else if (facing == LEFT)
        {
            (*Ycurr)--;
        }
    }
    else if ((*Xcurr + *Ycurr) % 2 != 0) // upside-down triangle
    {
        if (facing == DOWN)
        {
            (*Xcurr)++;
        }
        else if (facing == RIGHT)
        {
            (*Ycurr)++;
        }
        else if (facing == LEFT)
        {
            (*Ycurr)--;
        }
    }
    if (((*Xcurr + 1) < 0 || (*Xcurr + 1) > map->rows) || ((*Ycurr + 1) < 0 || (*Ycurr + 1) > map->cols))
    {
        // finish
        return 0;
    }
    return 1;
}

// Rpath - move right
void goRight(Direction *facing, int Xcurr, int Ycurr)
{
    if ((Xcurr + Ycurr) % 2 == 0)
    {
        if (*facing == RIGHT)
            *facing = RIGHT;
        else if (*facing == LEFT)
            *facing = FORWARD;
        else if (*facing == DOWN)
            *facing = LEFT;
    }
    else
    {
        if (*facing == RIGHT)
            *facing = DOWN;
        else if (*facing == FORWARD)
            *facing = RIGHT;
        else if (*facing == LEFT)
            *facing = LEFT;
    }
}

// Rpath - turning in a cell lefty
void turnLeft(Direction *facing, int Xcurr, int Ycurr)
{
    if ((Xcurr + Ycurr) % 2 == 0)
    {
        if (*facing == RIGHT)
            *facing = FORWARD;
        else if (*facing == FORWARD)
            *facing = LEFT;
        else if (*facing == LEFT)
            *facing = RIGHT;
    }
    else
    {
        if (*facing == RIGHT)
            *facing = LEFT;
        else if (*facing == LEFT)
            *facing = DOWN;
        else if (*facing == DOWN)
            *facing = RIGHT;
    }
}

// Lpath - move left
void goLeft(Direction *facing, int Xcurr, int Ycurr)
{
    if ((Xcurr + Ycurr) % 2 == 0)
    {
        if (*facing == RIGHT)
            *facing = FORWARD;
        else if (*facing == LEFT)
            *facing = LEFT;
        else if (*facing == DOWN)
            *facing = RIGHT;
    }
    else
    {
        if (*facing == RIGHT)
            *facing = RIGHT;
        else if (*facing == FORWARD)
            *facing = LEFT;
        else if (*facing == LEFT)
            *facing = DOWN;
    }
}

// Lpath - turning in a cell righty
void turnRight(Direction *facing, int Xcurr, int Ycurr)
{
    if ((Xcurr + Ycurr) % 2 == 0)
    {
        if (*facing == RIGHT)
            *facing = LEFT;
        else if (*facing == FORWARD)
            *facing = RIGHT;
        else if (*facing == LEFT)
            *facing = FORWARD;
    }
    else
    {
        if (*facing == RIGHT)
            *facing = DOWN;
        else if (*facing == LEFT)
            *facing = RIGHT;
        else if (*facing == DOWN)
            *facing = LEFT;
    }
}

// Right hand rule
void RightHand(Map *map, int startborder, int Xcurr, int Ycurr)
{
    Direction facing = (Direction)startborder;
    int cycleCounter = 0;
    Xcurr--;
    Ycurr--;

    while (!(Xcurr > map->rows || Ycurr > map->cols || Xcurr < 0 || Ycurr < 0))
    {
        if (!isborder(map, Xcurr, Ycurr, facing)) // checking for wall in front of me
        {
            printf("%d,%d\n", Xcurr + 1, Ycurr + 1);
            if (!moveForward(&Xcurr, &Ycurr, facing, map))
                break;
            goRight(&facing, Xcurr, Ycurr);
        }
        else
            turnLeft(&facing, Xcurr, Ycurr);

        cycleCounter++;
    }
}

// Left hand rule
void LeftHand(Map *map, int startborder, int Xcurr, int Ycurr)
{
    Direction facing = (Direction)startborder;
    int cycleCounter = 0;
    Xcurr--;
    Ycurr--;

    while (!(Xcurr > map->rows || Ycurr > map->cols || Xcurr < 0 || Ycurr < 0)) // checking for wall in front of me
    {
        if (!isborder(map, Xcurr, Ycurr, facing))
        {
            printf("%d,%d\n", Xcurr + 1, Ycurr + 1);
            if (!moveForward(&Xcurr, &Ycurr, facing, map))
                break;
            goLeft(&facing, Xcurr, Ycurr);
        }
        else
            turnRight(&facing, Xcurr, Ycurr);

        cycleCounter++;
    }
}

int main(int argc, char *argv[])
{
    enum PathType pathType;
    char *FileName;
    int Xstart = 0, Ystart = 0;

    if (argc < 2 || (argc == 2 && strcmp(argv[1], "--help") == 0))
    {
        getHelp();
        return 0;
    }
    else if (strcmp(argv[1], "--test") == 0)
    {
        if (argc != 3)
        {
            fprintf(stderr, "Usage: ./maze --test file.txt\n");
            return 1;
        }
        FileName = argv[2];
    }
    else if (argc == 5 && checkInput(argv[1], strtol(argv[2], NULL, 10), strtol(argv[3], NULL, 10), &pathType))
    {
        FileName = argv[4];
        Xstart = strtol(argv[2], NULL, 10);
        Ystart = strtol(argv[3], NULL, 10);
    }
    else
    {
        fprintf(stderr, "Invalid arguments. Use --help for usage information.\n");
        return 1;
    }

    FILE *bludiste = fopen(FileName, "r");
    if (bludiste == NULL)
    {
        fprintf(stderr, "No File/Failed to open file\n");
        return 1;
    }

    Map maze;

    int R = 0, C = 0;
    fscanf(bludiste, " %d %d ", &R, &C);
    if (R <= 0 || C <= 0)
    {
        printf("Invalid\n");
        return 0;
    }
    int valid = isMazeValid(bludiste, &maze, R, C);

    if ((strcmp(argv[1], "--test") == 0) && FileName != NULL)
    {
        if (valid)
        {
            printf("Valid\n");
            fclose(bludiste);
            free(maze.cells);
            maze.cells = NULL;
            return 0;
        }
        else
        {
            printf("Invalid\n");
            fclose(bludiste);
            free(maze.cells);
            maze.cells = NULL;
            return 1;
        }
    }

    int startborder = 0;
    switch (pathType)
    {
    case RPATH:
        startborder = start_border(&maze, Xstart, Ystart, pathType);
        RightHand(&maze, startborder, Xstart, Ystart);
        break;
    case LPATH:
        startborder = start_border(&maze, Xstart, Ystart, pathType);
        LeftHand(&maze, startborder, Xstart, Ystart);
        break;
    default:
        printf("Wrong pathtype.");
        break;
    }

    free(maze.cells);
    fclose(bludiste);
    return 0;
}
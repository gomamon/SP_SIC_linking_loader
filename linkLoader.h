enum record{
	recH, recD, recR, recT, recM ,recE
};

int ProgAddr(char addr[][COMMANDSIZE]);
int LoaderPass1(char obj_file[][COMMANDSIZE]);
int Run();
int BreakPoint(char addr[][COMMANDSIZE]);

int CheckObj(char filename[]);
int GetHeaderRec(FILE *fp, int cnt);

void InitEST();
int MakeExtSymNode(char sym[], int addr, int cnt);
void PrintEST();
int GetDefineRec(char str[], int cnt);

int LoaderPass2(char obj_file[][COMMANDSIZE]);
int GetTextRec(char str[], int cnt);

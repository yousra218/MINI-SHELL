#include "miniShell.h"

int	status = 0;

//Tableau des commandes internes avec leurs gestionnaires

t_builtin	g_builtin[] = 
{
	{.builtin_name = "echo", .foo=AB_echo},  
	{.builtin_name = "env", .foo=AB_env},    
	{.builtin_name = "exit", .foo=AB_exit},  
	{.builtin_name = NULL},                   
};


/**
 * AB_launch - Crée un processus enfant pour exécuter des commandes externes
 * @args: Tableau des arguments de la commande
 * Return: 1 en cas de succès, 0 en cas d'échec
 * Cas particuliers :
 * - args NULL : aucune action n'est effectuée
 * - Échec de fork : un message d'erreur est affiché, le shell continue
 * - Échec de execvp : le processus enfant se termine avec un code d'échec
 */
void	AB_launch(char **args){
	if(Fork() == 0)
		Execvp(args[0], args);
	else
		Wait(&status);
}

/**
 * AB_execute - Exécute des commandes internes ou externes
 * @args: Tableau des arguments de la commande
 * Return: 1 pour continuer le shell, 0 pour le quitter
 */
void	AB_execute(char **args)
{
	int i;
	const char *curr_builtin;

	if (!args || !args[0])
		return ;
	i = 0;

// 1) vérifier si une commande interne est demandée
	while((curr_builtin = g_builtin[i].builtin_name)){
		if(!strcmp(args[0], curr_builtin)){
		    status = (g_builtin[i].foo)(args);
		    return; 
		}
		i++;
	}
	
	AB_launch(args);
}

char *get_username() {
	struct passwd *pw = getpwuid(getuid());
	return pw ? pw->pw_name : "unknown";
}

int is_root() {
	return getuid() == 0;
}

char *AB_read_line() {
	char *line = NULL;
	size_t bufsize = 0;
	char cwd[BUFSIZ];

	// Obtenir le répertoire courant
	if (!getcwd(cwd, sizeof(cwd)))
		strcpy(cwd, "?");

	// Obtenir le nom d'utilisateur
	char *user = get_username();

	// Affichage du prompt coloré
	if (is_root()) {
		printf(RED "[%s" NC "👿" BLUE "%s" RED "] ># " NC, user, cwd);
	} else {
		printf(GREEN "[%s" NC "😃" CYAN "%s" GREEN "] >$ " NC, user, cwd);
	}

	// Lire la ligne entrée par l'utilisateur
	if (getline(&line, &bufsize, stdin) == -1) {
		perror("Erreur de lecture");
		free(line);
		return NULL;
	}

	return line;
}

/**
 * AB_split_line - Splits input line into tokens
 * @line: Input line to be tokenized
 * Return: Array of tokens or NULL on failure
 */
char	**AB_split_line(char *line)
{
	size_t bufsize;
	unsigned long position;
	char **tokens;

	bufsize = BUFSIZ;
	position = 0;
	tokens = Malloc(bufsize * sizeof *tokens);
	char *token;

	for(token = strtok(line, DL); token; token = strtok(NULL, DL)){
		tokens[position++] = token;	
		if(position >= bufsize){
			bufsize *= 2;
			tokens = Realloc(tokens, bufsize * sizeof *tokens);
		}
	}
	tokens[position] = NULL;
	return tokens;
}

int main(){
	printbanner();
	
	char	*line;
	char	**args;

	while((line = AB_read_line())){
		args = AB_split_line(line);	
		int i;
		if (args[0] && !strcmp(args[0], "cd")){
			if(args[1])
				 i = Chdir(args[1]);
			else
				chdir(getenv("HOME"));
		}
		else
			AB_execute(args);

		free(line);
		free(args);
	}

	return 0;
}

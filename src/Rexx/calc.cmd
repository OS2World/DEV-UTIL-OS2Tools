/* Rexx Calculator */

/* Variablen */
szCommand=""

/* Signale setzen */
SIGNAL ON SYNTAX NAME SyntaxProc
SIGNAL ON NOVALUE NAME SyntaxProc

SIGNAL ON ERROR NAME ExitHandler
SIGNAL ON FAILURE NAME ExitHandler
SIGNAL ON HALT NAME ExitHandler
SIGNAL ON NOTREADY NAME ExitHandler


/* Bearbeitungsschleife */
Call Title
do forever
  szCommand = GetCommand()
  call DoCommand szCommand
end



/* Eingabeprozedur */
GetCommand: procedure
  call Charout ,'>'
  pull szCommand
return szCommand


/* Commandointerpreter */
DoCommand: procedure
  arg szCommand

  select
    when szCommand = 'EXIT' then
      call ExitProc

    when szCommand = 'BYE' then
      call ExitProc

    when szCommand = 'QUIT' then
      call ExitProc

    when szCommand = 'HELP' then
      call HelpProc

    when szCommand = 'TIME' then
      call TimeDateProc

    when szCommand = 'DATE' then
      call TimeDateProc

    otherwise /* Unbekanntes Kommando -> interpretieren */
      szCommand='say "['||szCommand||'] = "||('||szCommand')'
    
      /* Nun das Kommando interpretieren */
      interpret szCommand        
  end
return 

/* Datum+Zeit des Systems ausgeben */
TimeDateProc: procedure
  say 'Systemzeit: 'date('L')' - 'time('N')
return

/* Exit */
ExitProc: procedure
  say 'Programmende.'
exit

/* ExitHandler */
ExitHandler:
  say 'Programmabbruch.'
exit


/* Syntax */
SyntaxProc: 
  say 'Syntaxfehler im Ausdruck.'
return


/* Titel ausgeben */
Title: procedure
  say '旼컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴커'
  say '� CALC.CMD - Minitaschenrechner in Rexx. (c) Patrick Haller 1995 �'
  say '읕컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴켸'
return

/* Hilfe */
HelpProc: procedure
  call Title
  say 'Hilfe zum Minitaschenrechner ...'
  say ''
  say 'Kommandoprompt:'
  say ' > [command]'
  say ''
  say '[command] kann jedes g걄tige Rexx-Commando sein. Alle Eingaben werden'
  say 'direkt an den Rexx-Interpreter geschickt.'
  say ''
  say 'Ausnahmen:'
  say '  HELP            Dieser kleine Hilfetext hier.'
  say '  EXIT,BYE,QUIT   Programm beenden.'
  say '  TIME,DATE       Aktuelle Systemzeit ausgeben.'
return
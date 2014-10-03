/* REXX-Script zum anzeigen und deregistrieren von WPS-Klassen */

call RxFuncAdd 'SysLoadFuncs', 'RexxUtil', 'SysLoadFuncs'
call SysLoadFuncs

parse arg argumente

if argumente='' then
do   /* Liste der Klassen ausgeben */
   say 'Suche alle registrierten WPS-Klassen'
   say '------------------------------------'
   call SysQueryClassList "list."
   do i = 1 to list.0
      say i'.' list.i
   end
end
else
do   /* Klasse deregistrieren */
  do i = 1 to words(argumente)
    szClass=word(argumente,i)

    if SysDeregisterObjectClass( szClass ) then
      say "Klasse ["||szClass||"] erfolgreich deregistriert"
    else
      say "Klasse ["||szClass||"] nicht erfolgreich deregistriert"
  end
end

/* REXX-Script zum Anzeigen und Registrieren von WPS-Klassen */

call RxFuncAdd 'SysLoadFuncs', 'RexxUtil', 'SysLoadFuncs'
call SysLoadFuncs

parse arg argObject argDLL

if argObject='' then
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
   if SysRegisterObjectClass( argObject, argDLL ) then
      say "Klasse " || argObject || " aus " || argDLL || " erfolgreich registriert."
   else
      say "Klasse " || argObject || " aus " || argDLL || " nicht erfolgreich registriert."
end

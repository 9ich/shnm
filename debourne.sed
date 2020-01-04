/^#$/d
s/\<LOCAL\>/static/g
s/\<PROC\>/extern/g
s/\<TYPE\>/typedef/g
s/\<STRUCT\>/typedef struct/g
s/\<UNION\>/typedef union/g
s/\<REG\>/register/g
s/\<THEN\>/){/g
s/\<ELSE\>/} else {/g
s/\<ELIF\>/} else if (/g
s/\<IF\>/if(/g
s/\<FI\>/;}/g
s/\<BEGIN\>/{/g
s/\<ENDSW\>/}/g
s/\<END\>/}/g
s/\<SWITCH\>/switch(/g
s/\<IN\>/){/g
s/\<FOR\>/for(/g
s/\<WHILE\>/while(/g
s/\<DONE\>/);/g
s/\<DO\>/){/g
s/\<OD\>/;}/g
s/\<REP\>/do{/g
s/\<PER\>/}while(/g
s/\<LOOP\>/for(;;){/g
s/\<POOL\>/}/g
s/\<SKIP\>/;/g
s,\<DIV\>,/,g
s/\<REM\>/%/g
s/\<NEQ\>/^/g
s/\<ANDF\>/\&\&/g
s/\<ORF\>/||/g

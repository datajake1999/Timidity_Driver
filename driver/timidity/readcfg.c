/*

TiMidity -- Experimental MIDI to WAVE converter
Copyright (C) 1995 Tuukka Toivonen <toivonen@clinet.fi>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

*/

#include <stdio.h>
#include <stdlib.h>

#ifndef _WIN32_WCE
#include <string.h>
#endif

#include "timid.h"

static int set_channel_flag(int32 *flags, int32 i, char *name)
{
    if (i==0) *flags=0;
    else if ((i<1 || i>16) && (i<-16 || i>-1))
    {
        fprintf(stderr,
        "%s must be between 1 and 16, or between -1 and -16, or 0\n",
        name);
        return -1;
    }
    else
    {
        if (i>0) *flags |= (1<<(i-1));
        else *flags &= ~ ((1<<(-1-i)));
    }
    return 0;
}

static int set_value(int32 *param, int32 i, int32 low, int32 high, char *name)
{
    if (i<low || i > high)
    {
        fprintf(stderr, "%s must be between %ld and %ld\n", name, low, high);
        return -1;
    }
    else *param=i;
    return 0;
}

#define MAXWORDS 10

int read_config_file(Timid *tm, char *name)
{
    FILE *fp;
    char tmp[1024], *w[MAXWORDS], *cp;
    ToneBank *bank=0;
    int i, j, k, line=0, words;
    static int rcf_count=0;
    
    if (rcf_count>50)
    {
        fprintf(stderr, "Probable source loop in configuration files");
        return (-1);
    }
    
    if (!(fp=open_file(tm, name, 1, OF_VERBOSE)))
		return -1;
    
    while (fgets(tmp, sizeof(tmp), fp))
    {
        line++;
        w[words=0]=strtok(tmp, " \t\r\n\240");
        if (!w[0] || (*w[0]=='#')) continue;
        while (w[words] && (words < MAXWORDS))
        w[++words]=strtok(0," \t\r\n\240");
        if (!strcmp(w[0], "dir"))
        {
            if (words < 2)
            {
                fprintf(stderr, "%s: line %d: No directory given\n", name, line);
                return -2;
            }
            tmp[0] = '\0';
            for (i=1; i<words; i++)
            {
                strcat(tmp, w[i]);
                if (i < words - 1)
                strcat(tmp, " ");
            }
            add_to_pathlist(tm, tmp);
        }
        else if (!strcmp(w[0], "source"))
        {
            if (words < 2)
            {
                fprintf(stderr, "%s: line %d: No file name given\n", name, line);
                return -2;
            }
            for (i=1; i<words; i++)
            {
                rcf_count++;
                read_config_file(tm, w[i]);
                rcf_count--;
            }
        }
        else if (!strcmp(w[0], "default"))
        {
            if (words != 2)
            {
                fprintf(stderr,
                "%s: line %d: Must specify exactly one patch name\n",
                name, line);
                return -2;
            }
            strncpy(tm->def_instr_name, w[1], 255);
            tm->def_instr_name[255]='\0';
        }
        else if (!strcmp(w[0], "drumset"))
        {
            if (words < 2)
            {
                fprintf(stderr, "%s: line %d: No drum set number given\n",
                name, line);
                return -2;
            }
            i=atoi(w[1]);
            if (i<0 || i>127)
            {
                fprintf(stderr,
                "%s: line %d: Drum set must be between 0 and 127\n",
                name, line);
                return -2;
            }
            if (!tm->drumset[i])
            {
                tm->drumset[i]=(ToneBank *)safe_malloc(sizeof(ToneBank));
                memset(tm->drumset[i], 0, sizeof(ToneBank));
            }
            bank=tm->drumset[i];
        }
        else if (!strcmp(w[0], "bank"))
        {
            if (words < 2)
            {
                fprintf(stderr, "%s: line %d: No bank number given\n",
                name, line);
                return -2;
            }
            i=atoi(w[1]);
            if (i<0 || i>127)
            {
                fprintf(stderr,
                "%s: line %d: Tone bank must be between 0 and 127\n",
                name, line);
                return -2;
            }
            if (!tm->tonebank[i])
            {
                tm->tonebank[i]=(ToneBank *)safe_malloc(sizeof(ToneBank));
                memset(tm->tonebank[i], 0, sizeof(ToneBank));
            }
            bank=tm->tonebank[i];
        }
        else {
            if ((words < 2) || (*w[0] < '0' || *w[0] > '9'))
            {
                fprintf(stderr, "%s: line %d: syntax error\n", name, line);
                return -2;
            }
            i=atoi(w[0]);
            if (i<0 || i>127)
            {
                fprintf(stderr, "%s: line %d: Program must be between 0 and 127\n",
                name, line);
                return -2;
            }
            if (!bank)
            {
                fprintf(stderr,
                "%s: line %d: Must specify tone bank or drum set "
                "before assignment\n",
                name, line);
                return -2;
            }
            if (bank->tone[i].name)
            free(bank->tone[i].name);
            strcpy((bank->tone[i].name=(char *)safe_malloc(strlen(w[1])+1)),w[1]);
            bank->tone[i].note=bank->tone[i].amp=bank->tone[i].pan=
            bank->tone[i].strip_loop=bank->tone[i].strip_envelope=
            bank->tone[i].strip_tail=-1;
            
            for (j=2; j<words; j++)
            {
                if (!(cp=strchr(w[j], '=')))
                {
                    fprintf(stderr, "%s: line %d: bad patch option %s\n",
                    name, line, w[j]);
                    return -2;
                }
                *cp++=0;
                if (!strcmp(w[j], "amp"))
                {
                    k=atoi(cp);

					if (k > MAX_AMPLIFICATION)
						k = MAX_AMPLIFICATION;

                    if ((k<0 || k>MAX_AMPLIFICATION) || (*cp < '0' || *cp > '9'))
                    {
                        fprintf(stderr,
                        "%s: line %d: amplification must be between "
                        "0 and %d\n", name, line, MAX_AMPLIFICATION);
                        return -2;
                    }
                    bank->tone[i].amp=k;
                }
                else if (!strcmp(w[j], "note"))
                {
                    k=atoi(cp);
                    if ((k<0 || k>127) || (*cp < '0' || *cp > '9'))
                    {
                        fprintf(stderr,
                        "%s: line %d: note must be between 0 and 127\n",
                        name, line);
                        return -2;
                    }
                    bank->tone[i].note=k;
                }
                else if (!strcmp(w[j], "pan"))
                {
                    if (!strcmp(cp, "center"))
                    k=64;
                    else if (!strcmp(cp, "left"))
                    k=0;
                    else if (!strcmp(cp, "right"))
                    k=127;
                    else
                    k=((atoi(cp)+100) * 100) / 157;
                    if ((k<0 || k>127) ||
                    (k==0 && *cp!='-' && (*cp < '0' || *cp > '9')))
                    {
                        fprintf(stderr,
                        "%s: line %d: panning must be left, right, "
                        "center, or between -100 and 100\n",
                        name, line);
                        return -2;
                    }
                    bank->tone[i].pan=k;
                }
                else if (!strcmp(w[j], "keep"))
                {
                    if (!strcmp(cp, "env"))
                    bank->tone[i].strip_envelope=0;
                    else if (!strcmp(cp, "loop"))
                    bank->tone[i].strip_loop=0;
                    else
                    {
                        fprintf(stderr, "%s: line %d: keep must be env or loop\n",
                        name, line);
                        return -2;
                    }
                }
                else if (!strcmp(w[j], "strip"))
                {
                    if (!strcmp(cp, "env"))
                    bank->tone[i].strip_envelope=1;
                    else if (!strcmp(cp, "loop"))
                    bank->tone[i].strip_loop=1;
                    else if (!strcmp(cp, "tail"))
                    bank->tone[i].strip_tail=1;
                    else
                    {
                        fprintf(stderr, "%s: line %d: strip must be env, loop, or tail\n",
                        name, line);
                        return -2;
                    }
                }
                else
                {
                    fprintf(stderr, "%s: line %d: bad patch option %s\n",
                    name, line, w[j]);
                    return -2;
                }
            }
        }
    }
    if (ferror(fp))
    {
        fprintf(stderr, "Can't read %s\n", name);
        close_file(fp);
        return -2;
    }
    close_file(fp);
    strncpy(tm->last_config, name, 1023);
    tm->last_config[1023]='\0';
    return 0;
}

/*
    genpeimg - Modify Portable Executable flags and properties.
    Copyright (C) 2009-2016  mingw-w64 project

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "img.h"

unsigned short set_pe_hdr_chara = 0;
unsigned short mask_pe_hdr_chara = 0xffff;
unsigned short set_pe_opt_hdr_dll_chara = 0;
unsigned short mask_pe_opt_hdr_dll_chara = 0xffff;
int dump_information = 0;
static char *file_name = NULL;

static void __attribute__((noreturn))
show_usage (void)
{
  fprintf (stderr, "genpeimg [options] files...\n");
  fprintf (stderr, "\nOptions:\n"
    " -p  Takes as addition argument one or more of the following\n"
    "     options:\n"
    "  +<flags> and/or -<flags>\n"
    "  flags are: l, r, n, s, u\n"
    "    l: large-address-aware (only valid for 32-bit)\n"
    "    r: removable-run-from-swap\n"
    "    n: new-run-from-swap\n"
    "    s: system\n"
    "    u: up-system-only\n");
  fprintf (stderr,
    " -d  Takes as addition argument one or more of the following\n"
    "     options:\n"
    "  +<flags> and/or -<flags>\n"
    "  flags are: d, f, n, i, s, b, a, w, t\n"
    "    d: dynamic base\n"
    "    f: force integrity\n"
    "    n: nx compatible\n"
    "    i: no-isolation\n"
    "    s: no-SEH\n"
    "    b: no-bind\n"
    "    a: app-container\n"
    "    w: WDM-driver\n"
    "    t: terminal-server-aware\n");
  fprintf (stderr,
    " -h: Show this page.\n"
    " -x: Dump image information to stdout\n");
  exit (0);
}

static int
pass_args (int argc, char **argv)
{
  int has_file = 0;
  int has_error = 0;
  while (argc-- > 0)
    {
      int is_pos = 1;
      char *h = *argv++;
      if (h[0] != '-')
        {
	  has_file = 1;
	  file_name = h;
	  continue;
	}
      switch (h[1])
        {
	case 'p':
	  if (h[2] != 0)
	    goto error_point;
	  if (argc == 0)
	    {
	      fprintf (stderr, "Missing argument for -p\n");
	      show_usage ();
	    }
	  h = *argv++; argc--;
	  while (*h != 0)
	    {
	      if (*h == '-') is_pos = 0;
	      else if (*h == '+') is_pos = 1;
	      else if (*h == 'l')
		{
		  if (is_pos)
		    set_pe_hdr_chara |= 0x20;
		  else
		    mask_pe_hdr_chara &= ~0x20;
		}
	      else if (*h == 'r')
		{
		  if (is_pos)
		    set_pe_hdr_chara |= 0x400;
		  else
		    mask_pe_hdr_chara &= ~0x400;
		}
	      else if (*h == 'n')
		{
		  if (is_pos)
		    set_pe_hdr_chara |= 0x800;
		  else
		    mask_pe_hdr_chara &= ~0x800;
		}
	      else if (*h == 's')
		{
		  if (is_pos)
		    set_pe_hdr_chara |= 0x1000;
		  else
		    mask_pe_hdr_chara &= ~0x1000;
		}
	      else if (*h == 'u')
		{
		  if (is_pos)
		    set_pe_hdr_chara |= 0x4000;
		  else
		    mask_pe_hdr_chara &= ~0x4000;
		}
	      else
		{
		  fprintf (stderr, "Unknown flag-option '%c' for -p\n", *h);
		  has_error = 1;
		}
	      ++h;
	    }
	  break;
	case 'd':
	  if (h[2] != 0)
	    goto error_point;
	  if (argc == 0)
	    {
	      fprintf (stderr, "Missing argument for -d\n");
	      show_usage ();
	    }
	  h = *argv++; argc--;
	  while (*h != 0)
	    {
	      switch (*h)
	        {
		case '-': is_pos = 0; break;
		case '+': is_pos = 1; break;
		case 'd':
		  if (is_pos) set_pe_opt_hdr_dll_chara |= 0x40;
		  else mask_pe_opt_hdr_dll_chara &= ~0x40;
		  break;
		case 'f':
		  if (is_pos) set_pe_opt_hdr_dll_chara |= 0x80;
		  else mask_pe_opt_hdr_dll_chara &= ~0x80;
		  break;
		case 'n':
		  if (is_pos) set_pe_opt_hdr_dll_chara |= 0x100;
		  else mask_pe_opt_hdr_dll_chara &= ~0x100;
		  break;
		case 'i':
		  if (is_pos) set_pe_opt_hdr_dll_chara |= 0x200;
		  else mask_pe_opt_hdr_dll_chara &= ~0x200;
		  break;
		case 's':
		  if (is_pos) set_pe_opt_hdr_dll_chara |= 0x400;
		  else mask_pe_opt_hdr_dll_chara &= ~0x400;
		  break;
		case 'b':
		  if (is_pos) set_pe_opt_hdr_dll_chara |= 0x800;
		  else mask_pe_opt_hdr_dll_chara &= ~0x800;
		  break;
		case 'a':
		  if (is_pos) set_pe_opt_hdr_dll_chara |= 0x1000;
		  else mask_pe_opt_hdr_dll_chara &= ~0x1000;
		  break;
		case 'w':
		  if (is_pos) set_pe_opt_hdr_dll_chara |= 0x2000;
		  else mask_pe_opt_hdr_dll_chara &= ~0x2000;
		  break;
		case 't':
		  if (is_pos) set_pe_opt_hdr_dll_chara |= 0x8000;
		  else mask_pe_opt_hdr_dll_chara &= ~0x8000;
		  break;
		default:
		  fprintf (stderr, "Unknown flag-option '%c' for -d\n", *h);
		  has_error = 1;
		  break;
		}
	      ++h;
	    }
	  break;
	case 'x':
	  if (h[2] == 0)
	    {
	      dump_information = 1;
	      break;
	    }
	  goto error_point;
	case 'h':
	  if (h[2] == 0)
	    show_usage ();
	  /* fallthru */
	default:
error_point:
	  fprintf (stderr, "Unknown option ,%s'\n", h);
	  has_error = 1;
	  break;
	}
    }
  if (has_error)
    show_usage ();
  if (!has_file)
    {
      fprintf (stderr, "File argument missing\n");
      show_usage ();
    }
  return 1;
}

int main (int argc, char **argv)
{
  pe_image *pe;
  --argc, ++argv;
  pass_args (argc, argv);

  pe = peimg_load (file_name);
  if (!pe)
    {
      fprintf (stderr, "File not found, or no PE-image\n");
      return 0;
    }

  if (dump_information)
    peimg_show (pe, stdout);
  /* First we need to do actions which aren't modifying image's size.  */
  peimg_set_hdr_characeristics (pe, set_pe_hdr_chara, mask_pe_hdr_chara);
  peimg_set_hdr_opt_dll_characteristics (pe, set_pe_opt_hdr_dll_chara, mask_pe_opt_hdr_dll_chara);
  if (pe->pimg->is_modified)
    pe->pimg->want_save = 1;
  peimg_free (pe);
  return 0;
}

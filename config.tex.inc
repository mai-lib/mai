
\ifdefined\target\else\def\target{\targetbook}\fi

\def\targetpreview{0}
\def\targetbook{1}
\def\targetfinal{2}

\ifcase\target %% pdf preview
% either:
%\def\geomopts{a4paper,margin=1in}
%\def\classopts{12pt,oneside}
% or:
\def\classopts{a4paper,12pt,oneside}
\def\geomopts{a4paper,margin=1in}
\def\codecolored{}
%
\or %% bookish
\def\classopts{a4paper,11pt,twoside}
\def\geomopts{a4paper,inner=25mm,outer=32mm,bottom=32mm,top=32mm}
\def\codecolored{}
\or %% final print
%% XXX ~20mm margin for punching a comb binding at this page count; swap inner and outer??
\def\classopts{a4paper,11pt,twoside}
\def\geomopts{a4paper,inner=25mm,outer=20mm,bottom=25mm,top=25mm}
\def\codecolored{}
%\def\doublespaced{}
\fi

\documentclass[\classopts]{book}

%% set pdf output size
%% either (nicer margins; 11pt is good with this; ymmv)
\ifdefined\geomopts
\usepackage[\geomopts]{geometry}
\else %% or
\setlength{\pdfpagewidth}{\paperwidth}
\setlength{\pdfpageheight}{\paperheight}
\fi
%%

%% OT1 encoding doesn't have an underscore
%% to avoid needing -filter esc-und.sh in noweave:
%% change encoding to T1, use a font designed for it (latin modern)
%% then make underscore a regular character in chunk names
%% \usepackage[T1]{fontenc}
%% \usepackage{lmodern}
%% \let\Osetupmodname=\setupmodname
%% \def\setupmodname{\Osetupmodname\catcode`\_=12}
%%

%% package introductions
\usepackage{natbib}
\usepackage{color}
\usepackage{verbatim}
\usepackage[hidelinks]{hyperref}
\usepackage{noweb}

%% noweb options
\noweboptions{noidentxref,smallcode,nomargintag,breakcode}
%% hyperref options
\hypersetup{colorlinks=false,linktoc=all,bookmarksnumbered=true}
%% natbib options
\bibpunct{(}{)}{;}{a}{,}{,}

%% assumes breakcode
%% fix bug in noweb: this should be parameterised
\nwbreakcodespace=32mm
\makeatletter
\def\nwdocspar{\@nwsemifilbreak{\nwbreakcodespace}}
\makeatother

%% testing alternative to breakcode - allow breaks both before and after code chunks
%% \let\nwenddocs=\filbreak
%% \let\nwdocspar=\filbreak

%% no initial indent after code chunks
%% note: requires no blank line between code ending @ and doc para
%% start; enforced by sed hax
\let\Onwdocspar=\nwdocspar
\def\nwdocspar{\Onwdocspar\noindent}

%% make code a distinct shade
\ifdefined\codecolored
\definecolor{code}{gray}{0.3}
\let\Onwenddeflinemarkup=\nwenddeflinemarkup
\def\nwenddeflinemarkup{\Onwenddeflinemarkup\color{code}}
\fi

%% 1.5-spaced text, single-spaced code
\ifdefined\doublespaced
\def\hack#1{\linespread{#1}\selectfont}
%% urgh. can't figure out why the vspace after a code chunk is so large
%% increase the vspace before it to (roughly) match, instead.
\let\Onwbegincode=\nwbegincode
\def\nwbegincode#1{\Onwbegincode{#1}\hack{1}\medskip}
\let\Onwendcode=\nwendcode
\def\nwendcode{\hack{1.5}\Onwendcode}
%% XXX we redefine chapter to begin the doublespace hack
%% this breaks the ToC, so do it after mainmatter in writeup.nw
\fi

%% custom subhead
%% XXX subsubsection* is ok except has lousy vspace when after a code chunk
\def\subhead#1{\noindent\textbf{#1}\hspace{.5em}}

%% \def\refC#1{Chapter~\ref{#1}}
%% \def\refS#1{Section~\ref{#1}}
%% \def\refU#1{Subsection~\ref{#1}}
% using autoref for this now
\def\chapterautorefname{Chapter}
\def\sectionautorefname{Section}
\def\subsectionautorefname{Subsection}

\setcounter{tocdepth}{1}

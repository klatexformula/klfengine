//
// This file was automatically generated with the docstrip utility.
//
// Copyright (C) 2020 by Philippe Faist, philippe.faist@bluewin.ch
// See LICENSE.txt for license information.
//

const char * klfimpl_sty_data =
  R"%%%###%%%###%%%(%% This is klfimpl.sty exported as C++ source code
\NeedsTeXFormat{LaTeX2e}[2005/12/01]
\ProvidesPackage{klfimpl}
    [2020/08/08 v0.2 klfimpl package]
\newbox\klf@eqnbox
\newdimen\klf@w
\newdimen\klf@h
\newdimen\klf@d
\newdimen\klf@th
\newdimen\klf@ppw
\newdimen\klf@pph
\newdimen\klf@hshift
\newdimen\klf@vshift
\newdimen\klf@em
\newdimen\klf@ex
\newdimen\klf@capxhgt
\newdimen\klf@dim@fixedwidth
\newdimen\klf@dim@fixedheight
\newdimen\klf@dim@topmargin
\newdimen\klf@dim@rightmargin
\newdimen\klf@dim@bottommargin
\newdimen\klf@dim@leftmargin
\newif\ifklf@ltxengine@latexdvi \klf@ltxengine@latexdvifalse
\newif\ifklf@ltxengine@pdflatex \klf@ltxengine@pdflatexfalse
\newif\ifklf@ltxengine@xelatex \klf@ltxengine@xelatexfalse
\newif\ifklf@ltxengine@lualatex \klf@ltxengine@lualatexfalse
\newif\ifklf@keeplayoutsizes
\klf@keeplayoutsizesfalse
\def\klf@set@fixedwidth{}
\def\klf@set@fixedheight{}
\def\klf@set@topmargin{0.1ex}
\def\klf@set@rightmargin{0.1ex}
\def\klf@set@bottommargin{0.1ex}
\def\klf@set@leftmargin{0.1ex}
\def\klf@set@xscale{1}
\def\klf@set@yscale{1}
\def\klf@set@xaligncoeff{0.5}
\def\klf@set@yaligncoeff{0.5}
\def\klf@set@topalignment{bbox}
\def\klf@set@bottomalignment{bbox}
\def\klf@set@baselineruletype{none}
\DeclareOption{latexdvi}{\klf@ltxengine@latexdvitrue}
\DeclareOption{pdflatex}{\klf@ltxengine@pdflatextrue}
\DeclareOption{xelatex}{\klf@ltxengine@xelatextrue}
\DeclareOption{lualatex}{\klf@ltxengine@lualatextrue}
\DeclareOption{keeplayoutsizes}{\klf@keeplayoutsizestrue}
\DeclareOption*{\PackageError{klfimpl}{Unknown option ‘\CurrentOption’}{}}
\ProcessOptions\relax
\def\klfSetFixedWidth#1{%
  \xdef\klf@set@fixedwidth{#1}}
\def\klfSetFixedHeight#1{%
  \xdef\klf@set@fixedheight{#1}}
\def\klfSetTopMargin#1{%
  \xdef\klf@set@topmargin{#1}}
\def\klfSetRightMargin#1{%
  \xdef\klf@set@rightmargin{#1}}
\def\klfSetBottomMargin#1{%
  \xdef\klf@set@bottommargin{#1}}
\def\klfSetLeftMargin#1{%
  \xdef\klf@set@leftmargin{#1}}
\def\klfSetXScale#1{%
  \xdef\klf@set@xscale{#1}%
  \ifx\klf@set@xscale\klf@macro@one
  \else
    \RequirePackage{graphics}%
  \fi
}
\def\klfSetYScale#1{%
  \xdef\klf@set@yscale{#1}%
  \ifx\klf@set@yscale\klf@macro@one
  \else
    \RequirePackage{graphics}%
  \fi
}
\def\klfSetScale#1{%
  \xdef\klf@set@xscale{#1}%
  \xdef\klf@set@yscale{#1}%
  \ifx\klf@set@xscale\klf@macro@one
  \else
    \RequirePackage{graphics}%
  \fi
}
\def\klf@macro@one{1}
\def\klfSetXAlignCoeff#1{%
  \xdef\klf@set@xaligncoeff{#1}%
}
\def\klfSetYAlignCoeff#1{%
  \xdef\klf@set@yaligncoeff{#1}%
}
\def\klfSetTopAlignment#1{%
  \xdef\klf@set@topalignment{#1}}
\def\klfSetBottomAlignment#1{%
  \xdef\klf@set@bottomalignment{#1}}
\def\klfSetBaselineRuleType#1{%
  \xdef\klf@set@baselineruletype{#1}}
\def\klf@ZeroLayoutSizes{%
  \oddsidemargin=\z@\relax
  \evensidemargin=\z@\relax
  \topmargin=\z@\relax
  \voffset=-1in\relax
  \hoffset=-1in\relax
  \headsep=\z@\relax
  \headheight=\z@\relax
  \marginparsep=\z@\relax
  \footskip=\z@\relax
  \parindent=\z@\relax
  \parskip=\z@\relax
  \topskip=\z@\relax
}
\def\klf@ZeroDisplaySkips{%
  \abovedisplayskip=\z@\relax
  \belowdisplayskip=\z@\relax
  \abovedisplayshortskip=\z@\relax
  \belowdisplayshortskip=\z@\relax
}
\ifklf@keeplayoutsizes
\else
  \klf@ZeroLayoutSizes
  \klf@ZeroDisplaySkips
\fi
\def\klfSetPaperSize#1#2{%
  \@tempdima=#1\relax
  \@tempdimb=#2\relax
  \klf@SetPaperSize@FromDims\@tempdima\@tempdimb
}
\def\klf@SetPaperSize@FromDims#1#2{%
  \global\textwidth=#1\relax
  \global\textheight=#2\relax
  \global\hsize=#1\relax
  \global\vsize=#2\relax
  \global\paperwidth=#1\relax
  \global\paperheight=#2\relax
  \ifklf@ltxengine@pdflatex
    \global\pdfpagewidth=#1\relax
    \global\pdfpageheight=#2\relax
  \fi
  \ifklf@ltxengine@xelatex
    \global\pdfpagewidth=#1\relax
    \global\pdfpageheight=#2\relax
  \fi
  \ifklf@ltxengine@lualatex
    \global\pagewidth=#1\relax
    \global\pageheight=#2\relax
  \fi
}
\def\klfcontent#1#2{%
  \unskip
  \samepage
  \setbox\klf@eqnbox=#1\bgroup
    \klf@ZeroDisplaySkips%
    #2%
    \global\klf@em=1em\relax
    \global\klf@ex=1ex\relax
    \setbox0=\hbox{X}%
    \global\klf@capxhgt=\ht0%
    \ifx\klf@set@fixedwidth\@empty\else
      \global\klf@dim@fixedwidth=\klf@set@fixedwidth\relax
    \fi
    \ifx\klf@set@fixedheight\@empty\else
      \global\klf@dim@fixedheight=\klf@set@fixedheight\relax
    \fi
    \global\klf@dim@topmargin=\klf@set@topmargin\relax
    \global\klf@dim@rightmargin=\klf@set@rightmargin\relax
    \global\klf@dim@bottommargin=\klf@set@bottommargin\relax
    \global\klf@dim@leftmargin=\klf@set@leftmargin\relax
}
\def\endklfcontent{%
  \egroup
  \klf@w=\wd\klf@eqnbox\relax
  \klf@h=\ht\klf@eqnbox\relax
  \klf@d=\dp\klf@eqnbox\relax
  \csname klf@correctboxheight@@\klf@set@topalignment\endcsname
  \csname klf@correctboxdepth@@\klf@set@bottomalignment\endcsname
  %\advance \klf@w 0.00001pt \relax
  %\advance \klf@h 0.00001pt \relax
  \klf@th=\klf@h\relax
  \advance \klf@th \klf@d \relax
  \ifx\klf@set@fixedwidth\@empty%
    \klf@ppw=\klf@w\relax
    \advance \klf@ppw \klf@dim@leftmargin \relax
    \advance \klf@ppw \klf@dim@rightmargin \relax
    \klf@hshift=\klf@dim@leftmargin\relax
  \else%
    \klf@ppw=\klf@dim@fixedwidth\relax
    \klf@hshift=\klf@set@xaligncoeff\klf@ppw\relax
    \advance \klf@hshift -\klf@set@xaligncoeff\klf@w\relax
    \advance \klf@hshift -\klf@set@xaligncoeff\klf@dim@rightmargin\relax
    \advance \klf@hshift -\klf@set@xaligncoeff\klf@dim@leftmargin\relax
    \advance \klf@hshift \klf@dim@leftmargin\relax
  \fi
  \ifx\klf@set@fixedheight\@empty%
    \klf@pph=\klf@th\relax
    \advance \klf@pph \klf@dim@topmargin \relax
    \advance \klf@pph \klf@dim@bottommargin \relax
    \klf@vshift=\klf@dim@topmargin\relax
  \else%
    \klf@pph=\klf@dim@fixedheight\relax
    \klf@vshift=\klf@set@yaligncoeff\klf@pph\relax
    \advance \klf@vshift -\klf@set@yaligncoeff\klf@th\relax
    \advance \klf@vshift -\klf@set@yaligncoeff\klf@dim@bottommargin\relax
    \advance \klf@vshift -\klf@set@yaligncoeff\klf@dim@topmargin\relax
    \advance \klf@vshift \klf@dim@topmargin\relax
  \fi
  \klf@RenderContentBox
  \klfDumpMetaInfo
  \ignorespaces
}
\def\klf@RenderContentBox{%
  \ifklf@ltxengine@latexdvi% tough luck
  \else
    \@tempdima=\klf@ppw
    \@tempdima=\klf@set@xscale\@tempdima\relax
    \@tempdimb=\klf@pph
    \@tempdimb=\klf@set@yscale\@tempdimb\relax
    \klf@SetPaperSize@FromDims\@tempdima\@tempdimb%
  \fi
  \let\klf@next\@firstofone
  \ifx\klf@set@xscale\klf@macro@one\else
    \let\klf@next\klf@do@scale\fi
  \ifx\klf@set@yscale\klf@macro@one\else
    \let\klf@next\klf@do@scale\fi
  \nobreak
  \klf@next{%
    \hbox to \z@{\vbox to \z@{%
        \hrule \@height\z@\nobreak
        \vskip \klf@vshift\relax\nobreak
        \hbox{\vrule \@width\z@ \relax
          \raise \klf@d \hbox to \z@{%
            \csname klf@baseline@rule@@\klf@set@baselineruletype\endcsname
          }%
          \hskip \klf@hshift\relax
          \raise \klf@d \box\klf@eqnbox}%
      }}%
  }%
}
\def\klf@do@scale#1{%
  \scalebox{\klf@set@xscale}[\klf@set@yscale]{#1}%
}
\def\klf@correctboxheight@@bbox{}
\def\klf@correctboxdepth@@bbox{}
\def\klf@correctboxheight@@Xheight{%
  \klf@h=\klf@capxhgt
}
\def\klf@correctboxdepth@@baseline{%
  \klf@d=\z@
}
\def\klf@baseline@rule@@none{}
\def\klfBaselineRuleLineSetup{}
\def\klfBaselineRuleLineThickness{0.05\p@}
\def\klf@baseline@rule@@line{%
  \begingroup
    \klfBaselineRuleLineSetup
    \vrule width\klf@ppw height\z@ depth\klfBaselineRuleLineThickness\relax
  \endgroup
}
\def\klfDumpMetaInfo{%
  \begingroup
    \klf@em=\klf@set@xscale\klf@em\relax
    \klf@ex=\klf@set@yscale\klf@ex\relax
    \klf@capxhgt=\klf@set@yscale\klf@capxhgt\relax
    \klf@ppw=\klf@set@xscale\klf@ppw\relax
    \klf@pph=\klf@set@yscale\klf@pph\relax
    \klf@hshift=\klf@set@xscale\klf@hshift\relax
    \klf@vshift=\klf@set@yscale\klf@vshift\relax
    \klf@w=\klf@set@xscale\klf@w\relax
    \klf@h=\klf@set@yscale\klf@h\relax
    \klf@d=\klf@set@yscale\klf@d\relax
    \klf@th=\klf@set@yscale\klf@th\relax
    \message{%
^^J%
***-KLF-META-INFO-BEGIN-***^^J%
EM={\the\klf@em}^^J%
EX={\the\klf@ex}^^J%
CAP_X_HEIGHT={\the\klf@capxhgt}^^J%
PAPER_WIDTH={\the\klf@ppw}^^J%
PAPER_HEIGHT={\the\klf@pph}^^J%
HSHIFT={\the\klf@hshift}^^J%
VSHIFT={\the\klf@vshift}^^J%
BOX_WIDTH={\the\klf@w}^^J%
BOX_HEIGHT={\the\klf@h}^^J%
BOX_DEPTH={\the\klf@d}^^J%
BOX_TOTALHEIGHT={\the\klf@th}^^J%
^^J%
***-KLF-META-INFO-END-***^^J%
    }
  \endgroup
}%
)%%%###%%%###%%%";

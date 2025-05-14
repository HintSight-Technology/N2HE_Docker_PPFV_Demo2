syntax on
set shortmess+=I
" Disable the default Vim startup message.
set shortmess+=I
"set relativenumber

" Always show the status line at the bottom, even if you only have one window open.
set laststatus=2

set hlsearch
" search while typing
set incsearch
" search is sensitive when include upper case
set ignorecase
set smartcase


" no error sound
set noerrorbells visualbell t_vb=

" latency of pressing esc
set ttimeout 
set ttimeoutlen=100

filetype plugin indent on
set completeopt=longest,menu
set autoindent
set shiftwidth=4
set expandtab
 	
colorscheme  murphy
set background=dark
set tabstop=4
set ruler
set number
set showcmd
set showmatch

" key mappings from learning VimScript the Hard Way
inoremap jk <esc>
inoremap <esc> <nop>

highlight Statement ctermfg=141
highlight Boolean ctermfg=14
hi ModeMsg term=bold ctermfg=116 
hi StatusLine term=bold ctermfg=141 ctermbg=None 
hi EndOfBuffer ctermfg=141 ctermbg=None
hi Function ctermfg=116
hi Type cterm=italic ctermfg=111
hi String cterm=italic ctermfg=9
hi Number ctermfg=141


" latex
hi texmath ctermfg=9
hi Special ctermfg=201 guifg=#ff00ff
hi texDelimiter ctermfg=116
hi special cterm=italic ctermfg=166
hi texSpecialChar ctermfg=116
hi texCmdName ctermfg=111
hi texGreek ctermfg=9

" Python
syntax match pythonFunction /\v[[:alpha:]_.]+\ze(\s?\()/
hi def link pythonFunction Function

" MarkDown
hi Title term=bold cterm=bold ctermfg=141

hi lineNr ctermfg=8
hi comment cterm=italic ctermfg=8
" create new highlight group
highlight MyTabSpace ctermfg=darkgrey
match MyTabSpace /\t\| /

hi Normal ctermfg=252 ctermbg=none

set nocompatible
set backspace=indent,eol,start
set statusline=%F%r\ %M\ %=%l,%v

nmap Q <Nop> 
" 'Q' in normal mode enters Ex mode. You almost never want this.

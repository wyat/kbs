/*
    Pirate Bulletin Board System
    Copyright (C) 1990, Edward Luke, lush@Athena.EE.MsState.EDU
    Eagles Bulletin Board System
    Copyright (C) 1992, Raymond Rocker, rocker@rock.b11.ingr.com
                        Guy Vega, gtvega@seabass.st.usm.edu
                        Dominic Tynes, dbtynes@seabass.st.usm.edu

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 1, or (at your option)
    any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

/* rrr - This is separated so I can suck it into the IRC source for use
   there too */

#include "modes.h"

char *
ModeType(mode)
int     mode;
{
    switch(mode) {
    case IDLE:      return "" ;
    case NEW:       return "斧头帮注册" ;
    case LOGIN:     return "进入斧头帮";
    case CSIE_ANNOUNCE:     return "汲取精华";
    case CSIE_TIN:          return "使用TIN";
    case CSIE_GOPHER:       return "使用Gopher";
    case MMENU:     return "主菜单";
    case ADMIN:     return "帮内管理";
    case SELECT:    return "选择讨论区";
    case READBRD:   return "浏览讨论区";
    case READNEW:   return "阅读新文章";
    case  READING:  return "阅读文章";
    case  POSTING:  return "发表文章" ;
    case MAIL:      return "信件选单" ;
    case  SMAIL:    return "寄信中";
    case  RMAIL:    return "读信中";
    case TMENU:     return "谈天说地区";
    case  LUSERS:   return "看谁在线上";
    case  FRIEND:   return "找线上好友";
    case  MONITOR:  return "监看中";
    case  QUERY:    return "查询帮友";
    case  TALK:     return "聊天" ;
    case  PAGE:     return "呼叫帮友" ;
    case  CHAT2:    return "梦幻国度";
    case  CHAT1:    return "盘丝洞中";
    case  CHAT3:    return "快哉亭"; 
    case  CHAT4:    return "老大聊天室"; 
    case  IRCCHAT:  return "会谈IRC";
    case LAUSERS:   return "探视帮友"; 
    case XMENU:     return "系统资讯";
    case  VOTING:   return "投票";
    case  BBSNET:   return "穿梭银河";
    case  EDITWELC: return "编辑 Welc";
    case EDITUFILE: return "编辑档案";
    case EDITSFILE: return "帮内管理";
    case  EDITSIG:  return "刻印";
    case  EDITPLAN: return "拟计画";
    case ZAP:       return "订阅讨论区";
    case EXCE_MJ:   return "围城争霸";
    case EXCE_BIG2: return "比大营";
    case EXCE_CHESS:return "楚河汉界";
    case NOTEPAD:   return "遗言板"; 
    case GMENU:     return "工具箱";
    case FOURM:     return "4m Chat";
    case ULDL:      return "UL/DL" ;
    case MSG:       return "丢斧头";
    case USERDEF:   return "自订参数";
    case EDIT:      return "修改文章";
    case OFFLINE:   return "自杀中..";
    case EDITANN:   return "编修精华";
    case WWW:       return "悠游 WWW"; 
    case CCUGOPHER: return "他站精华";
    case LOOKMSGS:  return "察看斧头";
    case WFRIEND:   return "寻人名册";
    case LOCKSCREEN:return "屏幕锁定";
    default: return "去了那里!?" ;
    }
}


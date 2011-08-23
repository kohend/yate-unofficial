/**
 * NamedList.cpp
 * This file is part of the YATE Project http://YATE.null.ro
 *
 * Yet Another Telephony Engine - a fully featured software PBX and IVR
 * Copyright (C) 2004-2006 Null Team
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "yateclass.h"

using namespace TelEngine;

static const NamedList s_empty("");

const NamedList& NamedList::empty()
{
    return s_empty;
}

NamedList::NamedList(const char* name)
    : String(name)
{
}

NamedList::NamedList(const NamedList& original)
    : String(original)
{
    for (const ObjList* l = original.m_params.skipNull(); l; l = l->skipNext()) {
	const NamedString* p = static_cast<const NamedString*>(l->get());
	m_params.append(new NamedString(p->name(),*p));
    }
}

NamedList::NamedList(const char* name, const NamedList& original, const String& prefix)
    : String(name)
{
    copySubParams(original,prefix);
}

NamedList& NamedList::operator=(const NamedList& value)
{
    String::operator=(value);
    clearParams();
    return copyParams(value);
}

void* NamedList::getObject(const String& name) const
{
    if (name == YSTRING("NamedList"))
	return const_cast<NamedList*>(this);
    return String::getObject(name);
}

NamedList& NamedList::addParam(NamedString* param)
{
    XDebug(DebugInfo,"NamedList::addParam(%p) [\"%s\",\"%s\"]",
        param,(param ? param->name().c_str() : ""),TelEngine::c_safe(param));
    if (param)
	m_params.append(param);
    return *this;
}

NamedList& NamedList::addParam(const char* name, const char* value, bool emptyOK)
{
    XDebug(DebugInfo,"NamedList::addParam(\"%s\",\"%s\",%s)",name,value,String::boolText(emptyOK));
    if (emptyOK || !TelEngine::null(value))
	m_params.append(new NamedString(name, value));
    return *this;
}

NamedList& NamedList::setParam(NamedString* param)
{
    XDebug(DebugInfo,"NamedList::setParam(%p) [\"%s\",\"%s\"]",
        param,(param ? param->name().c_str() : ""),TelEngine::c_safe(param));
    if (!param)
	return *this;
    ObjList* p = m_params.find(param->name());
    if (p)
	p->set(param);
    else
	m_params.append(param);
    return *this;
}

NamedList& NamedList::setParam(const char* name, const char* value)
{
    XDebug(DebugInfo,"NamedList::setParam(\"%s\",\"%s\")",name,value);
    NamedString *s = getParam(name);
    if (s)
	*s = value;
    else
	m_params.append(new NamedString(name, value));
    return *this;
}

NamedList& NamedList::clearParam(const String& name, char childSep)
{
    XDebug(DebugInfo,"NamedList::clearParam(\"%s\",'%1s')",
	name.c_str(),&childSep);
    String tmp;
    if (childSep)
	tmp << name << childSep;
    ObjList *p = &m_params;
    while (p) {
        NamedString *s = static_cast<NamedString *>(p->get());
        if (s && ((s->name() == name) || s->name().startsWith(tmp)))
            p->remove();
	else
	    p = p->next();
    }
    return *this;
}

// Remove a specific parameter
NamedList& NamedList::clearParam(NamedString* param)
{
    if (!param)
	return *this;
    ObjList* o = m_params.find(param);
    if (o)
	o->remove();
    XDebug(DebugInfo,"NamedList::clearParam(%p) found=%p",param,o);
    return *this;
}

NamedList& NamedList::copyParam(const NamedList& original, const String& name, char childSep)
{
    XDebug(DebugInfo,"NamedList::copyParam(%p,\"%s\",'%1s')",
	&original,name.c_str(),&childSep);
    if (!childSep) {
	// faster and simpler - used in most cases
	const NamedString* s = original.getParam(name);
	return s ? setParam(name,*s) : clearParam(name);
    }
    clearParam(name,childSep);
    String tmp;
    tmp << name << childSep;
    for (const ObjList* l = original.m_params.skipNull(); l; l = l->skipNext()) {
	const NamedString* s = static_cast<const NamedString*>(l->get());
        if ((s->name() == name) || s->name().startsWith(tmp))
	    addParam(s->name(),*s);
    }
    return *this;
}

NamedList& NamedList::copyParams(const NamedList& original)
{
    XDebug(DebugInfo,"NamedList::copyParams(%p) [%p]",&original,this);
    for (const ObjList* l = original.m_params.skipNull(); l; l = l->skipNext()) {
	const NamedString* p = static_cast<const NamedString*>(l->get());
	setParam(p->name(),*p);
    }
    return *this;
}

NamedList& NamedList::copyParams(const NamedList& original, ObjList* list, char childSep)
{
    XDebug(DebugInfo,"NamedList::copyParams(%p,%p,'%1s') [%p]",
	&original,list,&childSep,this);
    for (; list; list = list->next()) {
	GenObject* obj = list->get();
	if (!obj)
	    continue;
	String name = obj->toString();
	name.trimBlanks();
	if (name)
	    copyParam(original,name,childSep);
    }
    return *this;
}

NamedList& NamedList::copyParams(const NamedList& original, const String& list, char childSep)
{
    XDebug(DebugInfo,"NamedList::copyParams(%p,\"%s\",'%1s') [%p]",
	&original,list.c_str(),&childSep,this);
    ObjList* l = list.split(',',false);
    if (l) {
	copyParams(original,l,childSep);
	l->destruct();
    }
    return *this;
}

NamedList& NamedList::copySubParams(const NamedList& original, const String& prefix, bool skipPrefix)
{
    XDebug(DebugInfo,"NamedList::copySubParams(%p,\"%s\",%s) [%p]",
	&original,prefix.c_str(),String::boolText(skipPrefix),this);
    if (prefix) {
	unsigned int offs = skipPrefix ? prefix.length() : 0;
	for (const ObjList* l = original.m_params.skipNull(); l; l = l->skipNext()) {
	    const NamedString* s = static_cast<const NamedString*>(l->get());
	    if (s->name().startsWith(prefix)) {
		const char* name = s->name().c_str() + offs;
		if (*name)
		    addParam(name,*s);
	    }
	}
    }
    return *this;
}

void NamedList::dump(String& str, const char* separator, char quote, bool force) const
{
    if (force && str.null())
	str << separator;
    str << quote << *this << quote;
    const ObjList *p = m_params.skipNull();
    for (; p; p = p->skipNext()) {
        const NamedString* s = static_cast<const NamedString *>(p->get());
	String tmp;
	tmp << quote << s->name() << quote << "=" << quote << *s << quote;
	str.append(tmp,separator);
    }
}

int NamedList::getIndex(const NamedString* param) const
{
    if (!param)
	return -1;
    const ObjList *p = &m_params;
    for (int i=0; p; p=p->next(),i++) {
        if (static_cast<const NamedString *>(p->get()) == param)
            return i;
    }
    return -1;
}

int NamedList::getIndex(const String& name) const
{
    const ObjList *p = &m_params;
    for (int i=0; p; p=p->next(),i++) {
        NamedString *s = static_cast<NamedString *>(p->get());
        if (s && (s->name() == name))
            return i;
    }
    return -1;
}

NamedString* NamedList::getParam(const String& name) const
{
    XDebug(DebugInfo,"NamedList::getParam(\"%s\")",name.c_str());
    const ObjList *p = m_params.skipNull();
    for (; p; p=p->skipNext()) {
        NamedString *s = static_cast<NamedString *>(p->get());
        if (s->name() == name)
            return s;
    }
    return 0;
}

NamedString* NamedList::getParam(unsigned int index) const
{
    XDebug(DebugInfo,"NamedList::getParam(%u)",index);
    return static_cast<NamedString *>(m_params[index]);
}

const String& NamedList::operator[](const String& name) const
{
    const String* s = getParam(name);
    return s ? *s : String::empty();
}

const char* NamedList::getValue(const String& name, const char* defvalue) const
{
    XDebug(DebugInfo,"NamedList::getValue(\"%s\",\"%s\")",name.c_str(),defvalue);
    const NamedString *s = getParam(name);
    return s ? s->c_str() : defvalue;
}

int NamedList::getIntValue(const String& name, int defvalue, int minvalue, int maxvalue,
    bool clump) const
{
    const NamedString *s = getParam(name);
    return s ? s->toInteger(defvalue,0,minvalue,maxvalue,clump) : defvalue;
}

int NamedList::getIntValue(const String& name, const TokenDict* tokens, int defvalue) const
{
    const NamedString *s = getParam(name);
    return s ? s->toInteger(tokens,defvalue) : defvalue;
}

double NamedList::getDoubleValue(const String& name, double defvalue) const
{
    const NamedString *s = getParam(name);
    return s ? s->toDouble(defvalue) : defvalue;
}

bool NamedList::getBoolValue(const String& name, bool defvalue) const
{
    const NamedString *s = getParam(name);
    return s ? s->toBoolean(defvalue) : defvalue;
}

int NamedList::replaceParams(String& str, bool sqlEsc, char extraEsc) const
{
    int p1 = 0;
    int cnt = 0;
    while ((p1 = str.find("${",p1)) >= 0) {
	int p2 = str.find('}',p1+2);
	if (p2 > 0) {
	    String def;
	    String tmp = str.substr(p1+2,p2-p1-2);
	    tmp.trimBlanks();
	    int pq = tmp.find('$');
	    if (pq >= 0) {
		// param is in ${<name>$<default>} format
		def = tmp.substr(pq+1).trimBlanks();
		tmp = tmp.substr(0,pq).trimBlanks();
	    }
	    DDebug(DebugAll,"NamedList replacing parameter '%s' [%p]",tmp.c_str(),this);
	    const String* ns = getParam(tmp);
	    if (ns) {
		if (sqlEsc) {
		    const DataBlock* data = 0;
		    if (ns->null()) {
			NamedPointer* np = YOBJECT(NamedPointer,ns);
			if (np)
			    data = YOBJECT(DataBlock,np->userData());
		    }
		    if (data)
			tmp = data->sqlEscape(extraEsc);
		    else
			tmp = ns->sqlEscape(extraEsc);
		}
		else
		    tmp = *ns;
	    }
	    else
		tmp = def;
	    str = str.substr(0,p1) + tmp + str.substr(p2+1);
	    // advance search offset past the string we just replaced
	    p1 += tmp.length();
	    cnt++;
	}
	else
	    return -1;
    }
    return cnt;
}

/* vi: set ts=8 sw=4 sts=4 noet: */

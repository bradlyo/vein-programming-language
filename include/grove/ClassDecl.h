/*
** Copyright 2014-2015 Robert Fratto. See the LICENSE.txt file at the top-level
** directory of this distribution.
**
** Licensed under the MIT license <http://opensource.org/licenses/MIT>. This file
** may not be copied, modified, or distributed except according to those terms.
*/

#pragma once 

#include "Block.h"
#include "Named.h"
#include "Typed.h"
#include "TypeProvider.h"
#include "Accessible.h"

class MemberVarDecl;
class ClassMethod;

/**
 * Represents a class declaration.
 */
class ClassDecl : public Block, public Named, public Typed,
	public TypeProvider, public Accessible
{
protected:
	void createCtor(ClassMethod* method) const;
public:
	virtual ASTNode* copy() const override;
	
	virtual void resolve() override;
	virtual void build() override;
	
	virtual bool matchesType(Type* other) const override;
	
	virtual bool isAccessible() const override;
	
	std::vector<MemberVarDecl*> getMembers() const;
	std::vector<ClassMethod*> getMethods() const;
	
	/// Gets methods where the name is equal to the class name
	std::vector<ClassMethod*> getCtors() const;
	
	MemberVarDecl* getMember(OString& name) const;
	
	ClassDecl(OString name);
};
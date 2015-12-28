/*
** Copyright 2014-2015 Robert Fratto. See the LICENSE.txt file at the top-level
** directory of this distribution.
**
** Licensed under the MIT license <http://opensource.org/licenses/MIT>. This file
** may not be copied, modified, or distributed except according to those terms.
*/

#pragma once 

#include "Expression.h"

class DerefExpr : public Expression
{
private:
	Expression* m_expression = nullptr;
public:
	virtual ASTNode* copy() const override;
	
	virtual std::vector<ObjectBase**> getMemberNodes() override;
	
	virtual std::vector<std::vector<ObjectBase *>*> getMemberLists() override;
	
	Expression* getExpression() const;
	
	virtual bool hasPointer() const override;
	virtual llvm::Value* getPointer() const override;
	
	virtual void resolve() override;
	virtual void build() override;
	
	DerefExpr(Expression* expr);
	DerefExpr(const DerefExpr& other);
};
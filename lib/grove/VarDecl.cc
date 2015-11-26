/*
** Copyright 2014-2015 Robert Fratto. See the LICENSE.txt file at the top-level
** directory of this distribution.
**
** Licensed under the MIT license <http://opensource.org/licenses/MIT>. This file
** may not be copied, modified, or distributed except according to those terms.
*/

#include <grove/VarDecl.h>
#include <grove/Expression.h>

#include <grove/types/Type.h>
#include <grove/types/UIntType.h>

#include <util/assertions.h>
#include <util/llvmassertions.h>

#include <llvm/IR/IRBuilder.h>

llvm::Value* VarDecl::getValue() const
{
	assertExists(m_value, "m_value never initialized.");
	return IRBuilder()->CreateLoad(m_value);
}

llvm::Value* VarDecl::getPointer() const
{
	return m_value;
}

bool VarDecl::hasPointer() const
{
	return true;
}

Expression* VarDecl::getExpression() const
{
	return m_expr;
}

ASTNode* VarDecl::copy() const
{
	if (m_expr)
	{
		return new VarDecl(m_type, m_name, m_expr->copy()->as<Expression *>());
	}
	else
	{
		return new VarDecl(m_type, m_name, nullptr);
	}
}

void VarDecl::resolve()
{
	NamedSearchSettings settings;
	settings.searchWholeTree = false;
	if (findNamed(getName(), getType(), settings))
	{
		throw std::invalid_argument("Variable already exists.");
	}

	if (getType()->isVarTy() && getExpression() == nullptr)
	{
		throw std::runtime_error("A variable of type var must have \
								 an expression");
	}

	if (getExpression())
	{
		assertExists(getExpression()->getType(), "Expression has no type.");

    	if (getType()->isVarTy())
    	{
    		setType(getExpression()->getType());
    	}
	}
	
	if (getType()->isVariadiclySized())
	{
		for (auto size : getType()->getVariadicSizes())
		{
			if (size->getType()->isIntTy() == false)
			{
				throw std::runtime_error("variadic array type's sizes must all \
										 be integers.");
			}
		}
	}
}

void VarDecl::build()
{
	if (getType()->isVariadiclySized())
	{
		for (auto s : getType()->getVariadicSizes())
		{
			s->build();
			auto s_val = s->castTo(UIntType::get(64));
			
			if (m_size == nullptr)
			{
				m_size = s_val;
			}
			else
			{
				m_size = IRBuilder()->CreateMul(m_size, s_val);
			}
		}
	}
	
	setValue(IRBuilder()->CreateAlloca(getType()->getLLVMType(), m_size,
									   getName().str()));

	if (getExpression())
	{
		getExpression()->build();
		auto val = getExpression()->getValue();
		assertExists(val, "Built expression has no value.");

		val = getExpression()->castTo(getType());

		assertEqual<VAL, PTR>(val, getPointer(), "value does not match variable");

		IRBuilder()->CreateStore(val, getPointer());
	}
}

VarDecl::VarDecl(Type* type, std::string name, Expression* expression)
{
	assertExists(type, "Type must exist");

	if (type->isVoidTy())
	{
		throw std::runtime_error("type of variable cannot be void");
	}
	
	if (type->isVariadiclySized())
	{
		for (auto size : type->getVariadicSizes())
		{
			addChild(size, true);
		}
	}

	if (name == "")
	{
		throw std::invalid_argument("name cannot be empty");
	}

	m_name = name;
	m_expr = expression;

	setType(type);
	addChild(m_expr, false);
}

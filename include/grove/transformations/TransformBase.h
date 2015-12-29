/*
** Copyright 2014-2015 Robert Fratto. See the LICENSE.txt file at the top-level
** directory of this distribution.
**
** Licensed under the MIT license <http://opensource.org/licenses/MIT>. This file
** may not be copied, modified, or distributed except according to those terms.
*/

#pragma once 

class ASTNode;

class TransformBase
{
public:
	/// Transforms the AST.
	virtual void transform(ASTNode* root) const = 0;
};

extern void InitMemberAccessTransform();

void InitAllTransforms()
{
	InitMemberAccessTransform();
}
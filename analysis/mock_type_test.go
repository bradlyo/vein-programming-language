package analysis

import (
	"github.com/orange-lang/orange/ast"
	"github.com/orange-lang/orange/ast/types"
)

type mockNode struct{ nodeType types.Type }

func (*mockNode) IsNode()       {}
func (*mockNode) IsExpression() {}
func (*mockNode) IsIdentifier() {}
func (*mockNode) IsStatement()  {}

func (n *mockNode) Accept(visitor ast.NodeVisitor) {
	visitor.(*typeResolutionVisitor).Type = n.nodeType
}

func newMockNode(ty types.Type) *mockNode {
	return &mockNode{nodeType: ty}
}
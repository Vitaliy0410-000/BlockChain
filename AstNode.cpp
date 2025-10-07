#include "AstNode.h"

AstNode::AstNode(AstNodeType type, const std::string& value, int line)
    : type(type),value(value),line(line){}
AstNodeType AstNode::getType() const
{
    return type;
}
std::string AstNode::getValue() const
{
    return value;
}
const std::vector<std::unique_ptr<AstNode>>& AstNode::getChildren() const
{
    return children;
}
void AstNode::addChild(std::unique_ptr<AstNode> child)
{
    if(child!=nullptr)
    {
    children.push_back(std::move(child));
    }
}
std::unique_ptr<AstNode> AstNode::clone() const
{
     auto newNode = std::make_unique<AstNode>(type, value, line);
    for (const auto& child : children)
     {
        auto clonedChild = child->clone();
        newNode->addChild(std::move(clonedChild));
    }

    return newNode;
}

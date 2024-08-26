struct Node
{
    int value;
    Node* left;
    Node* right;
    Node(int i = 0) : value(i), left(nullptr), right(nullptr)
    {}
    ...
};

template<typename T, typename... TP>
Node* traverse(T np, TP... paths)
{
    return np->* ... ->* paths;
}

int main()
{
    // 初始化二叉树结构
    Node* root = new Node(0);
    root->left = new Node(1);
    root->left->right = new Node(2);
    ...

    // 遍历二叉树
    Node* node = traverse(root, left, right);

    ...
}
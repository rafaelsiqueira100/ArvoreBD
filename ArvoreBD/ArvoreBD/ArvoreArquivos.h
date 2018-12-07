#ifndef AVLDATABASE_H
#define AVLDATABASE_H
using namespace std;

#include <stdexcept>
#include <ios>
#include <iostream>
#include <fstream>

typedef struct Node {
	int valid;
	int key;
	int data_index;
	int balance;
	int left;
	int right;
} Node;

template <typename K, typename T>
class ArvoreArquivos
{
public:
	ArvoreArquivos(std::string data_path, std::string tree_path) {
		// Create files if it doesn't exist
		data_file.open(data_path, std::ios::app);
		tree_file.open(tree_path, std::ios::app);

		data_file.close();
		tree_file.close();

		// Open files for reading / writing
		data_file.open(data_path, std::ios::in | std::ios::out | std::ios::binary | std::ios::ate);
		tree_file.open(tree_path, std::ios::in | std::ios::out | std::ios::binary | std::ios::ate);

		if (tree_is_empty()) {
			write_root_pos(-1);
		}
	}

	~ArvoreArquivos() {
		// ...

		data_file.close();
		tree_file.close();
	}

	void add(const K &key, const T &info) {
		// If tree is empty, first insertion
		if (tree_is_empty()) {
			write_root_pos(write_data_node(key, info));
		}
		else {
			add_recursive(key, info, read_root_pos());
		}
	}

	void remove(const K &key) {
		// Throw exception if tree is empty
		if (tree_is_empty()) {
			throw std::invalid_argument("No info matches key passed to remove()");
		}

		int new_root_pos = remove_recursive(key, read_root_pos());
		write_root_pos(new_root_pos);
	}

	T get(const K &key) {
		return get_info_recursive(key, read_root_pos());
	}

	int get_height() {
		return get_node_height(read_root_pos());
	}

	bool tree_is_empty() {
		return tree_file.tellg() == 0 || read_root_pos() == -1;
	}
/*	friend ostream& operator<< (ostream&, const ArvoreBinariaAVL&) throw() {

	}*/
	ostream& print(std::ostream &os) {
		return print_recursive(os, read_root_pos(), 0);
		
	}
	
	


private:
	const int tree_file_offset = sizeof(int);

	std::fstream data_file;
	std::fstream tree_file;

	void add_recursive(const K &key, const T &info, int current_pos) {
		// Get current node
		Node node = read_node(current_pos);

		// Check current node key to find where to insert
		if (key == node.key) {
			throw std::invalid_argument("Info already on tree");
		}
		else if (key > node.key) {
			// Insert to right
			if (node.right == -1) {
				node.right = write_data_node(key, info);
				update_node(current_pos, node);
			}
			else {
				add_recursive(key, info, node.right);
			}
		}
		else if (key < node.key) {
			// Insert to left
			if (node.left == -1) {
				node.left = write_data_node(key, info);
				update_node(current_pos, node);
			}
			else {
				add_recursive(key, info, node.left);
			}
		}

		// Update node balance
		node.balance = get_node_balance(current_pos);
		update_node(current_pos, node);
		balance_node(current_pos);
	}

	int remove_recursive(const K &key, int current_pos) {
		if (current_pos == -1) {
			throw std::invalid_argument("Info not on tree");
		}

		Node node = read_node(current_pos);

		// If this node must be removed
		if (node.key == key) {
			if (node.left != -1) {
				Node biggest_node = read_node(get_biggest_node_pos(node.left));

				remove_recursive(biggest_node.key, current_pos);

				if (read_node(node.left).valid == -1) {
					node.left = -1;
				}

				node.key = biggest_node.key;
				node.data_index = biggest_node.data_index;
			}
			else if (node.right != -1) {
				Node smallest_node = read_node(get_smallest_node_pos(node.right));

				remove_recursive(smallest_node.key, current_pos);

				if (read_node(node.right).valid == -1) {
					node.right = -1;
				}

				node.key = smallest_node.key;
				node.data_index = smallest_node.data_index;
			}
			else {
				delete_node(current_pos);
				return -1;
			}
		}
		else if (key > node.key) {
			node.right = remove_recursive(key, node.right);
		}
		else if (key < node.key) {
			node.left = remove_recursive(key, node.left);
		}

		node.balance = get_node_balance(current_pos);
		update_node(current_pos, node);
		balance_node(current_pos);

		return current_pos;
	}

	K get_smallest_node_pos(int current_pos) {
		Node node = read_node(current_pos);
		if (node.left != -1) {
			return get_smallest_node_pos(node.left);
		}
		else {
			return current_pos;
		}
	}

	K get_biggest_node_pos(int current_pos) {
		Node node = read_node(current_pos);
		if (node.right != -1) {
			return get_biggest_node_pos(node.right);
		}
		else {
			return current_pos;
		}
	}

	T get_info_recursive(const K &key, int current_pos) {
		if (current_pos == -1) {
			throw std::invalid_argument("No info matches key passed to get_info()");
		}

		Node node = read_node(current_pos);
		if (key == node.key) {
			return read_data(node.data_index);
		}
		else if (key > node.key) {
			return get_info_recursive(key, node.right);
		}
		else if (key < node.key) {
			return get_info_recursive(key, node.left);
		}
	}

	int write_data_node(const K& key, const T& info) {
		int data_index = write_data(info);
		Node new_node = { 1, key, data_index, 0, -1, -1 };
		int node_index = write_node(new_node);
		return node_index;
	}

	void write_root_pos(int pos) {
		tree_file.clear();
		tree_file.seekp(0, std::ios::beg);
		tree_file.write(reinterpret_cast<char*>(&pos), sizeof(int));
		tree_file.flush();
	}

	int read_root_pos() {
		tree_file.clear();
		tree_file.seekg(0, std::ios::beg);
		int pos;
		tree_file.read(reinterpret_cast<char*>(&pos), sizeof(int));
		// tree_file >> pos;
		return pos;
	}

	int write_data(const T &data) {
		T* data_ptr = new T(data);

		tree_file.clear();
		data_file.seekp(0, std::ios::end);
		data_file.write(reinterpret_cast<char*>(data_ptr), sizeof(T));
		data_file.flush();

		return (data_file.tellg() / sizeof(T)) - 1;
	}

	T read_data(int pos) {
		T data;

		tree_file.clear();
		data_file.seekp(pos * sizeof(T), std::ios::beg);
		data_file.read(reinterpret_cast<char*>(&data), sizeof(T));

		return data;
	}

	int write_node(Node node) {
		Node* node_ptr = new Node(node);

		tree_file.clear();
		tree_file.seekp(0, std::ios::end);
		tree_file.write(reinterpret_cast<char*>(node_ptr), sizeof(Node));
		tree_file.flush();

		return ((int)(tree_file.tellg()) - tree_file_offset) / sizeof(Node) - 1;
	}

	Node read_node(int pos) {
		Node node;

		tree_file.clear();
		tree_file.seekg(tree_file_offset + pos * sizeof(Node), std::ios::beg);
		tree_file.read(reinterpret_cast<char*>(&node), sizeof(Node));

		return node;
	}

	void update_node(int pos, Node node) {
		Node* node_ptr = new Node(node);

		tree_file.clear();
		tree_file.seekp(tree_file_offset + pos * sizeof(Node), std::ios::beg);
		tree_file.write(reinterpret_cast<char*>(node_ptr), sizeof(Node));
		tree_file.flush();
	}

	void delete_node(int pos) {
		Node invalid_node = Node();
		invalid_node.valid = -1;
		update_node(pos, invalid_node);
	}

	void swap_nodes(int pos_a, int pos_b) {
		Node node_a;
		if (pos_a == -1) {
			node_a.valid = -1;
		}
		else {
			node_a = read_node(pos_a);
		}

		Node node_b;
		if (pos_b == -1) {
			node_b.valid = -1;
		}
		else {
			node_b = read_node(pos_b);
		}

		update_node(pos_a, node_b);
		update_node(pos_b, node_a);
	}

	int get_node_height(int pos) {
		if (pos == -1) {
			return 0;
		}

		Node node = read_node(pos);

		if (node.valid == -1) {
			return 0;
		}

		return max(get_node_height(node.right), get_node_height(node.left)) + 1;
	}

	int get_node_balance(int pos) {
		if (pos == -1) {
			return 0;
		}

		Node node = read_node(pos);

		return (get_node_height(node.right) - get_node_height(node.left));
	}

	bool balance_node(int pos) {
		Node node = read_node(pos);
		if (node.balance > 1) {
			if (read_node(node.right).balance < 0) {
				rotate_double_left(pos);
			}
			else {
				rotate_left(pos);
			}
		}
		else if (node.balance < -1) {
			if (read_node(node.left).balance > 0) {
				rotate_double_right(pos);
			}
			else {
				rotate_right(pos);
			}
		}
		else {
			return false;
		}
		return true;
	}

	void rotate_left(int pos) {
		Node old_root = read_node(pos);

		swap_nodes(pos, old_root.right);

		int old_root_pos = old_root.right;
		Node new_root = read_node(pos);

		int new_root_left_pos = new_root.left;

		new_root.left = old_root_pos;
		old_root.right = new_root_left_pos;

		// Adjust old and new root balances dynamically
		old_root.balance = old_root.balance - 1 - max(new_root.balance, 0);
		new_root.balance = new_root.balance - 1 + min(old_root.balance, 0);

		update_node(pos, new_root);
		update_node(old_root_pos, old_root);
	}

	void rotate_right(int pos) {
		Node old_root = read_node(pos);

		swap_nodes(pos, old_root.left);

		int old_root_pos = old_root.left;
		Node new_root = read_node(pos);

		int new_root_right_pos = new_root.right;

		new_root.right = old_root_pos;
		old_root.left = new_root_right_pos;

		// Adjust old and new root balances dynamically
		old_root.balance = old_root.balance + 1 - min(new_root.balance, 0);
		new_root.balance = new_root.balance + 1 + max(old_root.balance, 0);

		update_node(pos, new_root);
		update_node(old_root_pos, old_root);
	}

	void rotate_double_left(int pos) {
		Node node = read_node(pos);
		rotate_right(node.right);
		rotate_left(pos);
	}
	int min(int a, int b) {
		return (b < a) ? b : a;
	}
	int max(int a, int b) {
		return (b > a) ? b : a;
	}
	void rotate_double_right(int pos) {
		Node node = read_node(pos);
		rotate_left(node.left);
		rotate_right(pos);
	}

	ostream& print_recursive(std::ostream& os, int pos, int space) {
		if (pos == -1) {
			return os;
		}

		Node node = read_node(pos);

		if (node.valid == -1) {
			os << "nó não existe na árvore!"<< std::endl;
			return os;
		}

		space += 0;
		os << '{' <<
			'[' << "chave:" << node.key << "]" <<
			'[' << "valor:" << get(node.key) << "]";
		os << "[" << "    ";
		
		print_recursive(os, node.left, space);

		//os << std::endl;
		os <<"]"<<" , "<<'[' << "";
		print_recursive(os, node.right, space);

		os  << "]"  ;
		

		
		os << '}' ;
		return os;
	}
};

#endif
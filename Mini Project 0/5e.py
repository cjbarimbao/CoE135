class Node:
    def __init__(self, data = None):
        self.data = data
        self.next = None


class LinkedList:
    def __init__(self):
        self.head = None
    def append(self, node):
        current = self.head
        while (current != None):
            temp = current
            current = current.next
        temp.next = node
    def print_list(self):
        current = self.head
        print("List elements:", end = " ")
        while (current != None):
            print(current.data, end = " ")
            current = current.next
        print("")

llist = LinkedList()
llist.head = Node(1)
second = Node(2)
third = Node(3)
llist.head.next = second
second.next = third

llist.print_list()
fourth = Node(4)
llist.append(fourth)
llist.print_list()
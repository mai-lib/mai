/*
  untyped doubly-linked list abstraction

  _heavily_ inspired by include/linux/list.h,
  which is licensed under the

  GNU GENERAL PUBLIC LICENSE
     Version 2, June 1991
*/

typedef struct list
{
    struct list *next;
    struct list *prev;
} list_t;

#define LIST_INIT(_self) { &(_self), &(_self) }

static inline bool list_isempty(list_t *list)
{
    return list->next == list && list->prev == list;
}

static inline void list_init(list_t *list)
{
    list->next = list;
    list->prev = list;
}

static inline bool list_isfirst(list_t *head, list_t *list)
{
    return list->prev == head;
}
static inline bool list_islast(list_t *head, list_t *list)
{
    return list->next == head;
}

static inline void _list_set(list_t *prev, list_t *list, list_t *next)
{
    list->next = next;
    list->prev = prev;
    next->prev = list;
    prev->next = list;
}

static inline void list_add(list_t *prev, list_t *list)
{
    _list_set(prev, list, prev->next);
}

static inline void list_add_before(list_t *next, list_t *list)
{
    _list_set(next->prev, list, next);
}

static inline void list_remove(list_t *list)
{
    list->next->prev = list->prev;
    list->prev->next = list->next;
    list_init(list);
}

// splice constraint: from must be a head. to may be a link.
static inline void list_splice_before(list_t *to, list_t *from)
{
    if(list_isempty(from))
        return;
    from->prev->next = to;
    from->next->prev = to->prev;
    to->prev->next = from->next;
    to->prev = from->prev;
    list_init(from);
}

static inline void list_splice_after(list_t *to, list_t *from)
{
    if(list_isempty(from))
        return;
    from->prev->next = to->next;
    from->next->prev = to;
    to->next->prev = from->prev;
    to->next = from->next;
    list_init(from);
}

#define list_foreach(_head, _ptr)               \
    for(_ptr = (_head)->next;                   \
        _ptr != _head;                          \
        _ptr = _ptr->next)

#define list_foreach_from(_head, _ptr)          \
    for(;                                       \
        _ptr != _head;                          \
        _ptr = _ptr->next)

#define list_foreach_safe(_head, _ptr, _tmp)            \
    for(_ptr = (_head)->next, _tmp = _ptr->next;        \
        _ptr != _head;                                  \
        _ptr = _tmp, _tmp = _ptr->next)

#define list_foreach_reverse(_head, _ptr)       \
    for(_ptr = (_head)->prev;                   \
        _ptr != _head;                          \
        _ptr = _ptr->prev)

static inline size_t list_length(list_t *list)
{
    list_t *ptr;
    size_t count = 0;
    list_foreach(list, ptr)
        count++;
    return count;
}

/*
   slightly more convenient (albeit less standards-safe) interface
   inspired by mesa's gallium/auxiliary/util/u_double_list.h

   hic sunt demons nasal, if _obj is not initialised
*/
#define list_entry(_link, _obj, _mem)           \
    (void *)((char *)(_link)                    \
             - ((char *)&(_obj)->_mem           \
                - (char *)(_obj)))              \

#define list_foreach_entry(_head, _ptr, _mem)                           \
    for(_ptr = NULL, _ptr = list_entry((_head)->next, _ptr, _mem);      \
        &_ptr->_mem != (_head);                                         \
        _ptr = list_entry(_ptr->_mem.next, _ptr, _mem))

#define list_foreach_entry_reverse(_head, _ptr, _mem)                   \
    for(_ptr = NULL, _ptr = list_entry((_head)->prev, _ptr, _mem);      \
        &_ptr->_mem != (_head);                                         \
        _ptr = list_entry(_ptr->_mem.prev, _ptr, _mem))

#define list_foreach_entry_from(_head, _ptr, _mem)                      \
    for(; _ptr && &_ptr->_mem != (_head);                               \
        _ptr = list_entry(_ptr->_mem.next, _ptr, _mem))

#define list_foreach_entry_safe(_head, _ptr, _tmp, _mem)        \
    for(_ptr = NULL,                                            \
            _ptr = list_entry((_head)->next, _ptr, _mem),       \
            _tmp = list_entry(_ptr->_mem.next, _ptr, _mem);     \
        &_ptr->_mem != (_head);                                 \
        _ptr = _tmp,                                            \
            _tmp = list_entry(_ptr->_mem.next, _ptr, _mem))

#define list_while_entry(_head, _ptr, _mem)                             \
    for(_ptr = NULL, _ptr = list_entry((_head)->next, _ptr, _mem);      \
        &_ptr->_mem != (_head);                                         \
        _ptr = list_entry((_head)->next, _ptr, _mem))

/*
  simple singly-linked list abstraction
*/

#define SLIST(typ) typ *
#define slist_push(head, elt, link)             \
    ((elt)->link = (head), (head) = (elt))
#define slist_pop(head, link)                                   \
    ((head) ? _take_ptr((void **)&(head), (head)->link) : NULL)
#define slist_while_pop(head, ptr, link)        \
    while((ptr = slist_pop(head, link)))
#define slist_remove_head(head, link)           \
    ((head) = (head)->link)
#define slist_insert(before, after, link)                       \
    ((after)->link = (before)->link, (before)->link = (after))
#define slist_foreach(head, ptr, link)          \
    for(ptr = (head); ptr; ptr = ptr->link)
#define slist_foreach_safe(head, ptr, tmp, link)                \
    for(ptr = (head); ptr && (tmp = ptr->link, true); ptr = tmp)
#define slist_nreverse(head, link) do {                 \
        for(void *tmp = NULL, *ptr = (head); ptr; )     \
        {                                               \
            head = ptr;                                 \
            ptr = head->link;                           \
            head->link = tmp;                           \
            tmp = head;                                 \
        }                                               \
    } while(0)

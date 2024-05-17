#include "engine/atom.h"
#include "engine/context.h"
#include "engine/runtime.h"
#include "engine/type.h"
#include "util/list.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
struct _neo_atom {
  neo_type type;
  void *data;
  neo_list parents;
  neo_list children;
};

neo_atom create_neo_atom(neo_type type) {
  neo_atom atom = (neo_atom)malloc(sizeof(struct _neo_atom));
  atom->type = type;
  atom->data = NULL;
  atom->parents = create_neo_list(NULL);
  atom->children = create_neo_list(NULL);
  if (type) {
    size_t size = neo_type_get_size(type);
    if (size) {
      atom->data = malloc(size);
      memset(atom->data, 0, size);
    }
    neo_type_operator operator= neo_type_get_operator(type);
    if (operator.init) {
      operator.init(atom->data);
    }
  }
  return atom;
}

int check_neo_atom_is_alived(neo_atom atom) {
  if (!neo_list_length(atom->parents) && !atom->type) {
    return 0;
  }
  neo_list workflow = create_neo_list(NULL);
  neo_list cache = create_neo_list(NULL);
  neo_list_push(workflow, atom);
  int is_alived = 0;
  while (neo_list_length(workflow)) {
    neo_atom atom = (neo_atom)neo_list_pop(workflow);
    if (!atom) {
      continue;
    }
    if (neo_list_find(cache, atom) != NULL) {
      continue;
    }
    if (!neo_list_length(atom->parents) && !atom->type) {
      neo_list_push(cache, atom);
      is_alived = 1;
      break;
    }
    neo_list_node node = neo_list_head(atom->parents);
    while (node != neo_list_tail(atom->parents)) {
      neo_atom atom = (neo_atom)neo_list_node_get(node);
      if (atom) {
        neo_list_push(workflow, atom);
      }
      node = neo_list_node_next(node);
    }
  }
  free_neo_list(cache);
  free_neo_list(workflow);
  return is_alived;
}

void free_neo_atom(neo_atom atom) {
  if (!check_neo_atom_is_alived(atom)) {
    while (neo_list_length(atom->children)) {
      neo_list_node node = neo_list_node_next(neo_list_head(atom->children));
      neo_atom child = (neo_atom)neo_list_node_get(node);
      if (child) {
        neo_atom_remove_ref(child, atom);
      }
    }
    free_neo_list(atom->children);
    neo_list_node node = neo_list_node_next(neo_list_head(atom->parents));
    while (node != neo_list_tail(atom->parents)) {
      neo_atom parent = (neo_atom)neo_list_node_get(node);
      if (parent) {
        neo_list_node n = neo_list_find(parent->children, atom);
        neo_list_remove(parent->children, n);
      }
      node = neo_list_node_next(node);
    }
    free_neo_list(atom->parents);
    if (atom->data) {
      neo_type_operator operator= neo_type_get_operator(atom->type);
      if (operator.dispose) {
        operator.dispose(atom->data);
      }
      free(atom->data);
    }
    free(atom);
  }
}

int8_t neo_atom_set(neo_atom self, void *data) {
  neo_type_operator operator= neo_type_get_operator(self->type);
  if (operator.set) {
    return operator.set(self, data);
  }
  size_t size = neo_type_get_size(self->type);
  memcpy(self->data, data, size);
  return 1;
}
void *neo_atom_get(neo_atom self) {
  neo_type_operator operator= neo_type_get_operator(self->type);
  if (operator.get) {
    return operator.get(self);
  }
  return self->data;
}
neo_type neo_atom_get_type(neo_atom self) { return self->type; }

void neo_atom_add_ref(neo_atom self, neo_atom parent) {
  neo_list_push(self->parents, parent);
  neo_list_push(parent->children, self);
}
void neo_atom_remove_ref(neo_atom self, neo_atom parent) {
  neo_list_node node = neo_list_find(self->parents, parent);
  neo_list_remove(self->parents, node);
  node = neo_list_find(parent->children, self);
  neo_list_remove(parent->children, node);
  free_neo_atom(self);
}
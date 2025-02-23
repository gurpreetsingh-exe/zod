#include "engine/node_types.hh"

namespace zod {

auto NodeTree::add_link_partial(Node* from) -> NodeLink* {
  m_links.push_back(NodeLink());
  auto* link = &m_links[m_links.size() - 1];
  link->node_from = from;
  link->socket_from = &from->outputs.back();
  from->outputs[0].links.push_back(link);
  return link;
}

auto NodeTree::connect_link(NodeLink* link, Node* to) -> void {
  link->node_to = to;
  link->socket_to = &to->inputs.back();
  to->inputs[0].links.push_back(link);
}

auto NodeTree::add_link(Node* from, Node* to) -> NodeLink* {
  auto* link = add_link_partial(from);
  connect_link(link, to);
  return link;
}

auto NodeTree::remove_link(NodeLink* link) -> void {
  auto offset = link - m_links.data();
  auto it = m_links.begin() + offset;
  m_links.erase(it, it + 1);
}

} // namespace zod

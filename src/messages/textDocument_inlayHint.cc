// Copyright 2017-2018 ccls Authors
// SPDX-License-Identifier: Apache-2.0

#include "log.hh"
#include "message_handler.hh"
#include "pipeline.hh"
#include "query.hh"

#include <algorithm>
#include <variant>

namespace ccls {

namespace {
struct InlayHint {
  Position position{};
  InlayHintLabel label;
  std::optional<InlayHintKind> kind{};
  std::optional<std::vector<TextEdit>> textEdit{};
  std::optional<Tooltip> tooltip{};
  std::optional<bool> paddingLeft{};
  std::optional<bool> paddingRight{};
  // std::optional<LSPAny> data{};

  bool operator<(const InlayHint &o) const {
    return !(position == o.position) ? position < o.position : kind < o.kind;
  }
};

REFLECT_STRUCT(InlayHint, position, label, kind, textEdit, tooltip, paddingLeft,
               paddingRight);
} // namespace

[[maybe_unused]] void reflect(JsonWriter &visitor, InlayHintKind &value) {
  visitor.int64(static_cast<int64_t>(value));
}

[[maybe_unused]] void reflect(JsonWriter &visitor, Tooltip &value) {
  if (value.index() == 0) {
    reflect(visitor, std::get<std::string>(value));
  } else {
    auto markupContent = std::get<MarkupContent>(value);
    reflect(visitor, markupContent.value);
    reflect(visitor, markupContent.kind);
  }
}

[[maybe_unused]] void reflect(JsonWriter &visitor, MarkupKind &value) {
  visitor.int64(static_cast<int64_t>(value));
}

[[maybe_unused]] void reflect(JsonWriter &visitor, InlayHintLabel &value) {

  if (value.index() == 0) {
    reflect(visitor, std::get<std::string_view>(value));
  } else {
    auto inlayHintLabelPart = std::get<InlayHintLabelPart>(value);
    reflect(visitor, inlayHintLabelPart.value);
    reflect(visitor, inlayHintLabelPart.tooltip);
    reflect(visitor, inlayHintLabelPart.location);
  }
}

void MessageHandler::textDocument_inlayHint(InlayHintParam &param,
                                            ReplyOnce &reply) {
  int file_id;
  auto [file, wf] =
      findOrFail(param.textDocument.uri.getPath(), reply, &file_id);
  if (!wf)
    return;

  std::vector<InlayHint> result;
  std::vector<SymbolRef> syms =
      findSymbolsInLsRange(wf, file, param.range, false);

  for (const auto &sym : syms) {
    // if (std::none_of(syms.begin(), syms.end(), [&](auto &sym1) {
    //       return usr == sym1.usr && kind == sym1.kind;
    //     }))
    //   continue;
    InlayHint inlayHint;
    inlayHint.position.line = sym.range.start.line;
    inlayHint.position.character = sym.range.start.column;
    inlayHint.kind = sym.kind == Kind::Type || sym.kind == Kind::Var
                         ? InlayHintKind::Type
                         : InlayHintKind::Parameter;

    if (std::optional<SymbolInformation> info = getSymbolInfo(db, sym, true)) {
      switch (sym.kind) {
      case Kind::Type:
        inlayHint.label = "Type";
        break;
      case Kind::Var:
        inlayHint.label = "Var";
        break;
      case Kind::Func:
        inlayHint.label = "Other";
        break;
      }
    }

    result.push_back(inlayHint);
  }
  std::sort(result.begin(), result.end());
  reply(result);
}
} // namespace ccls

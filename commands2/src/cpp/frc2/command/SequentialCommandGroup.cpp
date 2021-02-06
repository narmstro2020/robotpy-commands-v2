// Copyright (c) FIRST and other WPILib contributors.
// Open Source Software; you can modify and/or share it under the terms of
// the WPILib BSD license file in the root directory of this project.

#include "frc2/command/SequentialCommandGroup.h"

using namespace frc2;

SequentialCommandGroup::SequentialCommandGroup(
    std::vector<std::shared_ptr<Command>>&& commands) {
  AddCommands(std::move(commands));
}

void SequentialCommandGroup::Initialize() {
  m_currentCommandIndex = 0;

  if (!m_commands.empty()) {
    m_commands[0]->Initialize();
  }
}

void SequentialCommandGroup::Execute() {
  if (m_commands.empty()) {
    return;
  }

  auto& currentCommand = m_commands[m_currentCommandIndex];

  currentCommand->Execute();
  if (currentCommand->IsFinished()) {
    currentCommand->End(false);
    m_currentCommandIndex++;
    if (m_currentCommandIndex < m_commands.size()) {
      m_commands[m_currentCommandIndex]->Initialize();
    }
  }
}

void SequentialCommandGroup::End(bool interrupted) {
  if (interrupted && !m_commands.empty() &&
      m_currentCommandIndex != invalid_index &&
      m_currentCommandIndex < m_commands.size()) {
    m_commands[m_currentCommandIndex]->End(interrupted);
  }
  m_currentCommandIndex = invalid_index;
}

bool SequentialCommandGroup::IsFinished() {
  return m_currentCommandIndex == m_commands.size();
}

bool SequentialCommandGroup::RunsWhenDisabled() const {
  return m_runWhenDisabled;
}

void SequentialCommandGroup::AddCommands(
    std::vector<std::shared_ptr<Command>>&& commands) {
  if (!RequireUngrouped(commands)) {
    return;
  }

  if (m_currentCommandIndex != invalid_index) {
    // wpi_setWPIErrorWithContext(CommandIllegalUse,
    throw std::runtime_error(
                               "Commands cannot be added to a CommandGroup "
                               "while the group is running");
  }

  for (auto&& command : commands) {
    command->SetGrouped(true);
    AddRequirements(command->GetRequirements());
    m_runWhenDisabled &= command->RunsWhenDisabled();
    m_commands.emplace_back(std::move(command));
  }
}

// Fill out your copyright notice in the Description page of Project Settings.


#include "AttackPlayerBTTaskNode.h"

#include "EnemyAIController.h"

EBTNodeResult::Type UAttackPlayerBTTaskNode::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	UBehaviorTreeComponent* BTComp = &OwnerComp;
	if(!BTComp)
		return EBTNodeResult::Failed;
	AEnemyAIController* BTController =
	 Cast<AEnemyAIController>(BTComp->GetOwner());
	if(!BTController)
		return EBTNodeResult::Failed;
	BTController->AttackPlayer();
	return EBTNodeResult::Succeeded;
}
